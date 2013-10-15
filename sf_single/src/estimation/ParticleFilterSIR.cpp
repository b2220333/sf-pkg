/**
 * @file 
 * @author Denise Ratasich
 * @date 11.10.2013
 *
 * @brief Implementation of the SIR particle filter.
 */

#include <ctime>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_real.hpp>

#include "estimation/ParticleFilterSIR.h"
#include "probability/pdfs.h"
#include "probability/sampling.h"

namespace estimation 
{
  ParticleFilterSIR::ParticleFilterSIR (unsigned int N) 
    : AbstractParticleFilter(N)
  {
    rng.seed((unsigned int)clock());
  }

  ParticleFilterSIR::~ParticleFilterSIR () 
  {
    // nothing to do
  }

  // -----------------------------------------
  // getters and setters
  // -----------------------------------------
  void ParticleFilterSIR::setProcessNoiseCovariance (MatrixXd& Q)
  {
    this->Q = Q;
    validated = false;
  }

  void ParticleFilterSIR::setMeasurementNoiseCovariance (MatrixXd& R)
  {
    this->R = R;
    validated = false;
  }

  void ParticleFilterSIR::validate (void)
  {
    try 
    {
      // check for minimal initialization -----------------------
      if (!f)			// callback empty
	throw std::runtime_error("State transition model missing.");
      if (!h)			// callback empty
	throw std::runtime_error("Observation model missing.");
      if (Q.rows() == 0)
	throw std::runtime_error("Process noise covariance missing.");
      if (R.rows() == 0)
	throw std::runtime_error("Measurement noise covariance missing.");
      if (particles[0].size() == 0)
	throw std::runtime_error("Particles not initialized.");      

      // take sizes from required parameters
      int n = particles[0].size();
      int m = R.rows();

      // create other parameters if missing ---------------------
      // none

      // check appropriate sizes of matrices and vectors --------
      if (Q.rows() != n  ||  Q.cols() != n)
	throw std::runtime_error("Process noise covariance has invalid size.");
      if (R.rows() != R.cols())
	throw std::runtime_error("Measurement noise covariance must be a square matrix.");

      // validation finished successfully -----------------------
      validated = true;

      // further things to initialize ---------------------------
      // create output state
      if (out.size() != n) {	// not initialized till now or invalid size from reinit
	out.clear();
	for (int i = 0; i < n; i++)
	  out.add(OutputValue());
      }
    } 
    catch(std::exception& e) 
    {
      std::string additionalInfo = "ParticleFilterSIR: Validation failed. ";
      throw estimator_error(additionalInfo + e.what());
    }
  }

  // -----------------------------------------
  // Overrides of ParticleFilterSIR's IEstimator implementation
  // -----------------------------------------

  void ParticleFilterSIR::serialize(std::ostream& os) const
  {
    os << "SIR ParticleFilter" << std::endl;
  }

  // -----------------------------------------
  // Particle Filtering
  // -----------------------------------------
  void ParticleFilterSIR::sample (void)
  {    
    // estimate next state of particles
    for (int i = 0; i < particles.size(); i++)
    {      
      // assume zero mean gaussian noise
      VectorXd w = probability::sampleNormalDistribution(VectorXd::Zero(Q.rows()), Q);
      
      f(particles[i], u);		// time update
      particles[i] = particles[i] + w;  // add noise
    }
  }

  void ParticleFilterSIR::weight (VectorXd z)
  {
    double sumWeights = 0;

    if (z.size() != R.rows())
      throw std::runtime_error("Size of measurement vector invalid.");

    // calculate weight
    for (int i = 0; i < particles.size(); i++)
    {
      // weight = likelihood of the measurement
      // estimate expected measurement when in state i
      VectorXd z_expected = VectorXd::Zero(z.size());
      h(z_expected, particles[i]);

      // get probability of the measurement (mean = z_expected,
      // covariance = measurement noise covariance)
      double weight = probability::pdfNormalDistribution(z, z_expected, R);
      
      weights[i] = weight;	// set weight
      sumWeights += weight;	// add to sum for normalization
    }

    // normalize weights
    for (int i = 0; i < particles.size(); i++)
    {
      weights[i] /= sumWeights;
    }
  }

  void ParticleFilterSIR::resample (void)
  {
    // choose N particles according their weight 
    int N = particles.size();
    std::vector<VectorXd> particles_new(N);

    // 1. generate CDF
    std::vector<double> cdf(N);
    cdf[0] = 0;
    for (int i = 1; i < N; i++)
      cdf[i] = cdf[i-1] + weights[i];

    // needed for random number generation
    boost::uniform_real<> dist(0,1);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<> >
      random(rng, dist);

    for (int i = 0; i < N; i++)
    {
      // 2. generate a random number between 0 and 1
      double c = random();

      // 3. check where the random number in the CDF fits -> this is the
      // sample to choose; a sample with higher weight has a higher span
      // in the CDF, hence will be chosen more likely
      int ci = 0;
      while (c > cdf[ci])
      	ci++;
    
      particles_new[i] = particles[i];
    }

    // copy lots of vectors! :( -> pointer for particles in class
    particles = particles_new;
  }
}
