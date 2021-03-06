/**
 * @file 
 * @author Denise Ratasich
 * @date 20.01.2014
 *
 * @brief Example configuration EKF for navigation.
 *
 * This configuration only uses the wheel velocities to estimate the
 * position. Angular velocity and velocity into heading direction can
 * be calculated directly out of the wheel velocities.
 */

// -----------------------------------------
// application specific defines and includes
// -----------------------------------------

#include <cmath>

// period in which the estimation should be done
#define ESTIMATION_PERIOD	(100)		// in ms

// must match the ESTIMATION_PERIOD
#define FILTER_T		(0.1)		// filter period T in s

// distance between wheels / 2 (in meters), evaluated with parameter
// estimation using the gyroscope
#define	RADIUS			(0.3)

// -----------------------------------------
// input
// -----------------------------------------

// Get the independent velocities of the wheels.
#define TOPICS_IN					\
  ((velocity_wheel_left) (std_msgs::Float32) (data))	\
  ((velocity_wheel_right) (std_msgs::Float32) (data))	\
  /**/

// The message includes.
#include <std_msgs/Float32.h>

// -----------------------------------------
// output
// -----------------------------------------

// Publish all state variables.
#define TOPICS_OUT			\
  ((x) (0))				\
  ((y) (1))				\
  ((th) (2))				\
  ((omega) (3))				\
  ((ds) (4))				\
  ((v) (5))				\
  /**/

// -----------------------------------------
// method and its parameters
// -----------------------------------------

// Use EKF for estimation.
#define METHOD	EXTENDED_KALMAN_FILTER

// State transition model.
//
// Translation and rotation are assumed to be constant, i.e. we don't
// know what the robot will drive next.
//
// x0: x = x + ds * cos(th)
// x1: y = y + ds * sin(th)
// x2: th = th + omega * T
// x3: omega = omega
// x4: ds = v * T
// x5: v = v
//
#define STATE_TRANSITION_MODEL			\
  ( x[0] + x[4]*std::cos(x[2])	)		\
  ( x[1] + x[4]*std::sin(x[2])	)		\
  ( x[2] + x[3]*FILTER_T	)		\
  ( x[3]			)		\
  ( x[5]*FILTER_T		)		\
  ( x[5]			)		\
  /**/

// Jacobian of STM to x.
// 
// df0/dx = (1) (0) (-ds*std::sin(th)) (0) (cos(th)) (0)
// df1/dx = (0) (1) (ds*std::cos(th)) (0) (sin(th)) (0)
// df2/dx = (0) (0) (1) (T) (0) (0)
// df3/dx = (0) (0) (0) (1) (0) (0)
// df4/dx = (0) (0) (0) (0) (T) (0)
// df5/dx = (0) (0) (0) (0) (0) (1)
#define STATE_TRANSITION_MODEL_JACOBIAN					\
  ( (1) (0) (-x[4]*std::sin(x[2])) (0)	      (std::cos(x[2])) (0) )	\
  ( (0) (1) (x[4]*std::cos(x[2]))  (0)	      (std::sin(x[2])) (0) )	\
  ( (0) (0) (1)			   (FILTER_T) (0)	       (0) )	\
  ( (0) (0) (0)			   (1)	      (0)	       (0) )	\
  ( (0) (0) (0)			   (0)	      (FILTER_T)       (0) )	\
  ( (0) (0) (0)			   (0)	      (0)	       (1) )	\
  /**/

// Observation model.
//
// The wheel velocities are published in standard format of rosaria,
// i.e. mm/s. Hence these values have to be converted to m/s.
//
#define OBSERVATION_MODEL			\
  ( (x[5] - x[3]*RADIUS)*1000 )			\
  ( (x[5] + x[3]*RADIUS)*1000 )			\
  /**/

// Jacobian of OM to x.
//
// dh0/dx = (0) (0) (0) (-R*1000) (0) (1000)
// dh1/dx = (0) (0) (0) (R*1000) (0) (1000)
#define OBSERVATION_MODEL_JACOBIAN		\
  ( (0) (0) (0) (-RADIUS*1000) (0) (1000) )	\
  ( (0) (0) (0) (RADIUS*1000)  (0) (1000) )	\
  /**/

// Process noise covariance.
// 
// guess...
// max velocity = 0.7m/s
// max slope (rosaria) = 0.03m/s per 100ms
//
#define PROCESS_NOISE_COVARIANCE		\
  ( (0) (0) (0) (0) (0) (0) )			\
  ( (0) (0) (0) (0) (0) (0) )			\
  ( (0) (0) (0) (0) (0) (0) )			\
  ( (0) (0) (0) (0.5) (0) (0) )			\
  ( (0) (0) (0) (0) (0) (0) )			\
  ( (0) (0) (0) (0) (0) (0.3) )		\
  /**/

// The variance of the sensor, i.e. our measurement.
// 
// deviation (according test runs): 2mm/s
//
#define MEASUREMENT_NOISE_COVARIANCE		\
  ( (2) (0) )					\
  ( (0) (2) )					\
  /**/

// optional
// Initialize with start position (0).
#define INITIAL_STATE				\
  ( 0 )						\
  ( 0 )						\
  ( 0 )						\
  ( 0 )						\
  ( 0 )						\
  ( 0 )						\
  /**/
