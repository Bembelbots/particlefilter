# Particle Filter for Self-Localization 

This repository contains the implementation of a particle filter for Self Localization in the [Standard Platform League](https://spl.robocup.org) in RoboCup, a test program to run the filter, an example logfile with inputdata and a program to vizualize such logfiles.
The particle filter is written in C++, the visualization in Python 2.7.

A particle filter is an Monte Carlo localization algorithm to estimate the pose of a robot with visual perceived landmarks and odometry while having a given map. 

The code was developed for the usage in the Standard Platform League in RoboCup by the **Bembelbots Team**. Some more information about the particle filter could be found in the Bembelbots TeamResearchReport for 2019 (see https://bembelbots.de/web/robocup/publications/).


# Usage

## Initialization
The particle filter (src/particlefilter.{h,cpp}) must be initialized with 

* a Map of type PlayingField, containing the position of the landmarks of the field: lines, L-,T-,X-crossed, center-circle and penaltymarks. And Positions for special situations: Initial State (fixed Positions for 5 robots), unpenalized and manual placement 

Additional possible configuration Parameters:

* numParticles: the number of particles
* odoStdev: the expected error of your odometry
* startMCS: the starting position of your odometry system, normally (x=0,y=0,orientation=0)
* startPosition: the Position at which the particles are initialized
* robot_id, kickoff, role: all affect the positions to which filter is set in certain situations(e.g initial-state, manual placement)

## Update
The particle filter can be updated with:
```  cpp
/* param
 * visionResults :with the visual perceived landmarks (Lines, L-,T-,X-Crossed, Midcircle and Penaltymarks)
 * the current position of MCS(startMCS+ all movements since initialization), 
 * hypos: poseEstimates e.g. information from teammate, extracted positions form further landmark processing etc., 
*/
void update(const std::vector<VisionResult> &visionresult,
                DirectedCoord odometry,
                const std::pair<std::vector<DirectedCoord>,int> &hypos);
``` 

The visionResults and odometry are essential for the particle filter. Last year we also added the pose estimates/hypotheses that readjust the particles (details in our Team Reasearch Report). To test the filter with and without additional hypos we provide to logfile within this repo: jrlGT.log, jrlGTwHypos.log

the filtered position can then be read with `get_position()` 

## Events
The particle filter behaves different in different situations/gamephases which could be send to the filter by events:

* EV_INTIAL,
* EV_LOST_GROUND,
* EV_BACK_UP,
* EV_FALLEN,
* EV_PENALIZED,
* EV_UNPENALIZED,
* EV_STATE_INITIAL,
* EV_STATE_READY,
* EV_STATE_SET,
* EV_STATE_PLAYING,
* EV_STATE_FINISHED

In the Initial state, particles are spread around initial position which are depending on the robots id. In the set state the robot is sensetiv for manual placement. After penalized particles are spread at the possible positions on the sidelines.

# Requirements
The C++ code can be compiled with a C++14 compatible compiler (GCC and Clang have been tested). To build the test program, [cmake](https://cmake.org) is required.
The LogFileVizualizer requires python 2.7 and PyQt4.


# Example

We provide a test program and a logfile which you can use to run the particle filter.

**Compile the program** with cmake and run: (if no logfile is provided the different initializations are saved in the logfile)

``` 
./particlefiltertest <logfile> 
``` 


**To vizualize** use the LogFileVizualizer(test/LogFileVizualizer/vizualizer.py) (additionally the fieldsize could be set with -s  default:0 = JRL; 1  = SPL)
``` 
python vizualizer.py -l <logfile> 
``` 


**To test the filter** on the provided logfile(test/jrlGT.log) run 
``` 
./runTest.sh test/jrlGT.log

``` 

# License

Copyright (c) 2020 Bembelbots. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

 3. The end-user documentation included with the redistribution, if any, must include the following acknowledgment: "This product includes software developed by Bembelbots (www.bembelbots.de)." Alternately, this acknowledgment may appear in the software itself, if and wherever such third-party acknowledgments normally appear.

 4. For each Bembelbots code release from which parts are used in a RoboCup competition, the usage shall be announced in the SPL mailing list (currently robocup-nao@cc.gatech.edu) one month before the first competition in which you are using it. The announcement shall name which parts of this code are used.

 5. Bug fixes regarding existing code shall be sent back to Bembelbots via GitHub pull requests (https://github.com/Bembelbots).

THIS SOFTWARE IS PROVIDED BY BEMBELBOTS ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NAO-TEAM HTWK NOR ITS MEMBERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
