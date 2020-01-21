/**
 * @author Module owner: Bembelbots Frankfurt, sinaditzel
 *
 *
 */

#include <array>
#include <iostream>
#include <stdlib.h>
#include <constants.h>
#include <algorithm>

#include "particlefilter.h"

using namespace std;

Particle::Particle(const DirectedCoord &data, const float &w)
    : weight(w), pose(data) {}


void Particle::setParticle(const DirectedCoord &data, const float &w) {
    pose = data;
    weight = w;
}

Feature::Feature(){};

Feature::Feature(const int type, const float dist, const float angle,
                 const float orientation = 0.0f, const int id = 0)
    : type(type), dist(dist), angle(angle), orientation(orientation),id(id){}

// get my default initial parameters
ParticleFilter::Settings::Settings(PlayingField *pf) :
    pf(pf),numParticles(80),
    odoStdev(DirectedCoord(0.01f, 0.01f, 0.005f)),
    robot_id(1), has_kickoff(1), role(RobotRole::STRIKER)
     {}


ParticleFilter::ParticleFilter(const Settings &config):
    conf(config),pos(DirectedCoord(0.0f, 0.0f, 0.0f)),confidence (0.0f),
    lastMcsPosition(config.startMcsPosition), 
    isFallenRobot(false), isReplaced(false),isPenalized(false),
    penalizedGamestate(GameState::INITIAL),gamestate(GameState::INITIAL){    
    
    //initialize particels
    for (uint i = 0; i < conf.numParticles; ++i) {
        particles.push_back(Particle(conf.startPosition, 1.0f/conf.numParticles));
    }
    //place particles at both sides
    initHandler();
    //State events
    handle_event(EV_STATE_INITIAL, function<void()>(
                     bind(&ParticleFilter::initHandler, this)));
    handle_event(EV_STATE_READY, function<void()>(
                     bind(&ParticleFilter::readyHandler, this)));
    handle_event(EV_STATE_SET, function<void()>(
                     bind(&ParticleFilter::setHandler, this)));
    handle_event(EV_STATE_PLAYING, function<void()>(
                     bind(&ParticleFilter::playHandler, this)));
    handle_event(EV_PENALIZED, function<void()>(
                     bind(&ParticleFilter::penalizedHandler, this)));
    handle_event(EV_UNPENALIZED, function<void()>(
                     bind(&ParticleFilter::unpenalizedHandler, this)));
    //lost ground event
    handle_event(EV_FALLEN,[&](){isFallenRobot = true;});
    //back on ground event
    handle_event(EV_BACK_UP, function<void()>(
                     bind(&ParticleFilter::standUpHandler, this)));
}
ParticleFilter::~ParticleFilter() {}

void ParticleFilter::handle_event(const tLocalizationEvent &ev,
                                std::function<void()> handler) {
    ev_callbacks[ev] = handler;
}

void ParticleFilter::emit_event(const tLocalizationEvent &ev) {
    if (conf.pf == NULL) {
        return;
    }
    if (ev_callbacks.find(ev) != ev_callbacks.end()) {
        ev_callbacks.at(ev)();
    } else {
        cout << "Localization event: " << (int) ev << " is not handled!";
    }
}

//spread particles around initial position of the robot
void ParticleFilter::initHandler() {
    gamestate = GameState::INITIAL;
    vector<DirectedCoord> inits = {conf.pf->getInitialPose().at(conf.robot_id)};
    float deviation = conf.pf->_lengthInsideBounds/50.0f;   
    setParticlesToPosition(inits , deviation);
    pos = particles.at(0).pose;
}


void ParticleFilter::playHandler() {
    gamestate = GameState::PLAYING;
}

void ParticleFilter::readyHandler() {
    if (gamestate == GameState:: INITIAL) {
        initHandler();
    }
    gamestate = GameState::READY;
}
void ParticleFilter::setHandler() {
    penaltyKickHandler();   // robots for penalty kick are placed in set state
    gamestate = GameState::SET;
}

//spread particles on both sides at the boundary in our half
void ParticleFilter::penalizedHandler() {
    penalizedGamestate = gamestate;
    isPenalized = true;
}

//spread particles on both sides at the boundary in our half
void ParticleFilter::unpenalizedHandler() {
    isPenalized = false;

    // in unpenalized the both possible positions(on the both sidelines the rbot could be placed after
    // penalized ) are saved, particles are spread around this positions in turn
    vector<DirectedCoord> possible_pos = conf.pf->getUnpenalizedPose();
    float deviationX = conf.pf->_lengthInsideBounds/10.0f; 
    //if "motion in set" -penalty robot is places around current position
    if (penalizedGamestate == GameState:: SET){
        possible_pos.push_back(pos);
    }
    setParticlesToPosition(possible_pos , deviationX);

    //filter invalid positions
    for (auto &particle: particles) {
        while((particle.pose.coord.x) < ((- conf.pf->_lengthInsideBounds/2 )+0.2)) {
            particle.pose.coord.x += 0.5;
        }
    }
    pos = particles.at(0).pose;
    penaltyKickHandler();
}

//spread particles equaly on position with given deviation
void ParticleFilter::setParticlesToPosition(vector<DirectedCoord> positions, 
                                                    float deviationX , float deviationY,
                                                    float deviationAlpha, float amount) {
    normal_distribution <float> distributionX(0.0f, deviationX);
    normal_distribution <float> distributionY(0.0f, deviationY);
    normal_distribution <float> distributionAlpha(0.0f, deviationAlpha);
    amount = max(amount*conf.numParticles, 1.f);
    for (uint i_particle = 0 ; i_particle < amount; i_particle++){
        int i_pos = i_particle % positions.size();
        particles.at(i_particle).pose = DirectedCoord(positions.at(i_pos).coord.x + distributionX(generator),
                              positions.at(i_pos).coord.y + distributionY(generator),
                              positions.at(i_pos).angle.rad + distributionAlpha(generator));
        particles.at(i_particle).weight = 1.0f/conf.numParticles;
    }
}

void ParticleFilter::standUpHandler() {
    isFallenRobot = false;
    if (gamestate == GameState::INITIAL){
        initHandler();
    }
    if (gamestate == GameState::SET){
        manualPlacementHandler();
    }
}

//spread particles at possible positions wihle manual placement
void ParticleFilter::manualPlacementHandler() {
    if (!isPenalized){
        isReplaced = true;
        
        float deviationX = 0.15f; 
        float deviationY = 0.2f; 
        bool isGoali= (conf.role == RobotRole::GOALKEEPER);
        vector<DirectedCoord> man_placement_pose = conf.pf->getManualPlacementPose(true,isGoali); 
        man_placement_pose.push_back(pos);
        setParticlesToPosition(man_placement_pose, deviationX,deviationY);
        pos = particles.at(0).pose;

        penaltyKickHandler();
    }
}

// calculate particle positions for general penalty kick challenge
// does nothing if role is neither PENALTYKICKER nor PENALTYGOALIE
void ParticleFilter::penaltyKickHandler() {
    if (conf.role == RobotRole::PENALTYKICKER) {

        constexpr auto errorX{0.2f}, errorY{0.2f}, errorA{0.2f};
        static const auto penaltyMarkPos = DirectedCoord(conf.pf->getPenaltyMarkPosition(true), 0);
        vector<DirectedCoord> penaltykickerPos = {(penaltyMarkPos - DirectedCoord(1.0f,0.0f,0.0f))};
        static const array<Angle, 6> angles{{0, 0, -60, -30, 30, 60}};
        /*//FOR PENALTYKICKERCHALLENGE
        for (const auto &a: angles) {
            penaltyMarkPos.angle.degree = M_PI-a;
            penaltykickerPos.push_back(penaltyMarkPos.walk(DirectedCoord(1.0f, 0.0f, 0.0f)));*/
        setParticlesToPosition(penaltykickerPos, errorX, errorY, errorA);    

    } else if(conf.role == RobotRole::PENALTYGOALIE) {
        // goalie always starts in the center of his own goal
        // use normal distribution for error, with greater error along Y than X
        float errorX{0.1f}, errorY{conf.pf->_goalWidth/5.f}, errorA{0.2f};
        DirectedCoord goaliPos(-conf.pf->_lengthInsideBounds/2.f, 0, 0);
        setParticlesToPosition({goaliPos}, errorX, errorY, errorA);
    }
    pos = particles.at(0).pose;
}


//function gets mcs state and move particles
void ParticleFilter::moveParticles(const DirectedCoord &currMcsPosition) {
    //cout << isReplaced<< (gamestate == GameState:: INITIAL) <<isPenalized<<isFallenRobot<<endl;
    if ((isReplaced) or (gamestate == GameState:: INITIAL) or (isPenalized)) {
        // if robot has been replaced ignore odometry
        lastMcsPosition = currMcsPosition;
        isReplaced = false;
        return;
    }
    if (isFallenRobot) {
        return;
    }
    //caculate odometry: difference between the last MCS_Position and the actual MCS_ Position
    //use the toRCS function to get the differenc in RCS Coordinate System
    DirectedCoord odometry = (currMcsPosition - lastMcsPosition);
    odometry.coord = Coord(currMcsPosition.toRCS(lastMcsPosition).coord);
    lastMcsPosition = DirectedCoord(currMcsPosition);

    //assume error of odometry is normal distributed
    normal_distribution<double> erroralpha(0.0f, conf.odoStdev.coord.x);
    normal_distribution<double> errorx(0.0f, conf.odoStdev.coord.y);
    normal_distribution<double> errory(0.0f, conf.odoStdev.angle.rad);

    //if robot has moved, move particles
    if ((odometry.coord.x != 0.0f) or (odometry.coord.y != 0.0f)
            or (abs(odometry.angle.rad)> 0.0001f)) {
        // particle 0 is moved according to the odometry without error
        /*particles.at(0).pose = particles.at(0).pose.walk(DirectedCoord(
                                   odometry.coord.x, odometry.coord.y, odometry.angle.rad));*/
        for (auto &particle: particles) {
            // the odometry is only zero, if we are'nt moving, then the error is also zero
            float x = (odometry.coord.x != 0.0f) ? (odometry.coord.x + errorx(
                    generator)):0.0f;
            float y = (odometry.coord.y != 0.0f) ? (odometry.coord.y + errory(
                    generator)):0.0f;
            float angle =(abs(odometry.angle.rad)> 0.0001f) ? (odometry.angle.rad
                         +erroralpha(generator)):0.0f;
            // move particle accoring to the calculatet odometry+error
            // use walk function to rotate Coords to the WCS Coordinate system.
            particle.pose = particle.pose.walk(DirectedCoord(x, y, angle));
        }
    }
}

/*
calculate a probability for one visionResult by comparing it with the landmarks from the same type
*/
std::pair<float,Feature> ParticleFilter::calculateProbabilityOfMatchingLandmark(
    Feature visionresult, vector<Feature> pfLandmarks) {
    // calculate probability by comparing angle and distance
    float probability = 0.0f;
    Feature matchedLandmark;
    for (auto pfLandmark: pfLandmarks) {

        assert(visionresult.type == pfLandmark.type);

        float distance = visionresult.dist - pfLandmark.dist;
        
        float angle;
        if ((pfLandmark.dist > 0.1f) and (visionresult.dist > 0.1f)) {
            angle = fabsf(visionresult.angle - pfLandmark.angle);
            angle = min(angle, 2.0f* M_PI_F - angle);
        } else {
            // if there is no distance between robot and landmark, the angle calculation will not work.
            angle = 0.0f ;
        }

        // beside the angle from the robot to landmark, the landmark has an orientation.
        // e.g the orientation of a TCross is the direction of the line that crosses the other line centric
        // see in BembelbotsTeamResearchReport for Robocup 2019
        float orientationdist;
        if (JSVISION_LINE == visionresult.type){
            orientationdist = Angle(visionresult.orientation).dist(pfLandmark.orientation).rad;
            orientationdist = min(fabsf(orientationdist), fabsf(Angle(orientationdist - M_PI_F).rad)); //for lines the direction is not clear :/
        }
        else{
            orientationdist = fabsf(Angle(visionresult.orientation).dist(pfLandmark.orientation).rad);
        }

        float tmpProb = prob(distance) * prob(angle)* prob(orientationdist);
        
        //choose the highest probability
        if (tmpProb > probability) {
            matchedLandmark = pfLandmark;
            probability = tmpProb;
        }
    }
    return {probability, matchedLandmark};

}
/*
calculate new weights of particles by comparing the visionresults with the conf.pf landmarks
returns if weighting of particles was sucsessful

*/
bool ParticleFilter::measurementModel(const vector<VisionResult> &vrs) {
    bool isMeasurementUpdate = false;
    if (!vrs.empty()) {
        vector<Feature> vrsGoals;
        vector<Feature> vrsLines;
        vector<Feature> vrsLCrosses;
        vector<Feature> vrsTCrosses;
        vector<Feature> vrsXCrosses;
        vector<Feature> vrsCircles;
        //sort visionresults by type (line, crosses,goals)
        //extract dist and angle of visionresults and save typedepending Features
        for (size_t x = 0; x < vrs.size(); x++) {
            if (vrs.at(x).type == JSVISION_LINE) {
                //for a line, choose closest point for distance and angle calculation
                Coord lineStart(vrs.at(x).rcs_x1, vrs.at(x).rcs_y1);
                Coord lineEnd(vrs.at(x).rcs_x2, vrs.at(x).rcs_y2);
                if (lineStart.dist(lineEnd) > conf.pf->_penaltyLength+0.1f) {//discard short lines
                    float orientation = (lineEnd - lineStart).direction();
                    Coord line = Coord(0.0f, 0.0f).closestPointOnLine(lineStart, lineEnd);
                    vrsLines.push_back(Feature(JSVISION_LINE, line.dist(), line.angle().rad,
                                        orientation));
                }
            }
            /*if (vrs.at(x).type == JSVISION_GOAL) {
                vrsGoals.push_back(Feature(JSVISION_GOAL, vrs.at(x).rcs_distance,
                                           vrs.at(x).rcs_alpha));
            }*/
            if (vrs.at(x).type == JSVISION_LCROSS) {
                vrsLCrosses.push_back(Feature(JSVISION_LCROSS, vrs.at(x).rcs_distance,
                                              vrs.at(x).rcs_alpha, vrs.at(x).extra_float));
            }
            if (vrs.at(x).type == JSVISION_TCROSS) {
                vrsTCrosses.push_back(Feature(JSVISION_TCROSS, vrs.at(x).rcs_distance,
                                              vrs.at(x).rcs_alpha, vrs.at(x).extra_float));
            }
            if (vrs.at(x).type == JSVISION_XCROSS) {
                vrsXCrosses.push_back(Feature(JSVISION_XCROSS, vrs.at(x).rcs_distance,
                                              vrs.at(x).rcs_alpha, vrs.at(x).extra_float));
            }
            if (vrs.at(x).type == JSVISION_CIRCLE) {
                vrsCircles.push_back(Feature(JSVISION_CIRCLE, vrs.at(x).rcs_distance,
                                            vrs.at(x).rcs_alpha));
            }
        }

        vector<Feature> pfLines;
        vector<Feature> pfLCrosses;
        vector<Feature> pfTCrosses;
        vector<Feature> pfXCrosses;
        vector<Feature> pfGoals;
        vector<Feature> pfCircles;
        for (auto &particle: particles) {
            matchedLandmarks.clear();
            float probability = 1.0f;
            // for every particle compare Visionresulst with Landmarks from Playingfield
            if (!vrsLines.empty()) {
                pfLines = createLineFeature(particle);
                for (auto &vrsLine: vrsLines) {
                    auto res = calculateProbabilityOfMatchingLandmark(vrsLine, pfLines);
                    probability *= res.first;
                    matchedLandmarks.push_back(res.second);
                }
            }
            if (!vrsLCrosses.empty()) {
                pfLCrosses = createLCrossFeature(particle);
                for (auto &vrsLCross: vrsLCrosses) {
                    auto res = calculateProbabilityOfMatchingLandmark(vrsLCross, pfLCrosses);
                    probability *= res.first;
                    matchedLandmarks.push_back(res.second);
                }
            }
            if (!vrsTCrosses.empty()) {
                pfTCrosses = createTCrossFeature(particle);
                for (auto &vrsTCross: vrsTCrosses) {
                    auto res = calculateProbabilityOfMatchingLandmark(vrsTCross, pfTCrosses);
                    probability *= res.first;
                    matchedLandmarks.push_back(res.second);
                }
            }
            if (!vrsXCrosses.empty()) {
                pfXCrosses = createXCrossFeature(particle);
                for (auto &vrsXCross: vrsXCrosses) {
                    auto res = calculateProbabilityOfMatchingLandmark(vrsXCross, pfXCrosses);
                    probability *= res.first;
                    matchedLandmarks.push_back(res.second);
                }
            }
            if (!vrsCircles.empty()) {
                pfCircles = createCircleFeature(particle);
                for (auto &vrsCircle: vrsCircles) {
                    auto res = calculateProbabilityOfMatchingLandmark(vrsCircle, pfCircles);
                    probability *= res.first;
                    matchedLandmarks.push_back(res.second);
                }
            }
            // if we see one visionresult probability is smaller then 1.0
            if (probability != 1.0f) {
                particle.weight = probability;
                isMeasurementUpdate = true;
            }
        }
    }
    return isMeasurementUpdate;
}


//https://people.eecs.berkeley.edu/~pabbeel/cs287-fa11/slides/particle-filters++_v2.pdf
void ParticleFilter::lowVarianzeResample() {
    uniform_real_distribution<double> uniform_distribution(0.0,
            1.0f/particles.size());
    double random_number =  uniform_distribution(generator);

    //because we can't work in place on particles, we need some new array to save them
    vector<Particle> tmp_particles;
    tmp_particles.reserve(conf.numParticles);
    float beam = 0.f;
    for (size_t i = 0; i<(conf.numParticles); i++) {
        // every particle is according to a number(/area of numbers):
        // 1. first calculating this number
        beam = random_number + (i*(1.0f/conf.numParticles));
        // 2. choose particle
        size_t count_particle = 0;
        for (float sum_weight = particles.at(0).weight; (sum_weight < beam)
                and (count_particle < conf.numParticles-1);
                sum_weight += particles.at(count_particle).weight) {
            count_particle++;
        }
        tmp_particles.emplace_back(Particle(DirectedCoord(particles.at(
                                       count_particle).pose.coord.x,
                                   particles.at(count_particle).pose.coord.y,
                                   particles.at(count_particle).pose.angle.rad),
                                   1.0f/conf.numParticles));
    }
    //copy the choosen particles from temp to particles
    particles = move(tmp_particles);
}


void ParticleFilter::calculatePose() {
    //sum up all positions and divide by particle size to get the mean position
    DirectedCoord mean(particles.at(0).pose);
    for (size_t i = 1; i<(particles.size()); i++) {
        mean.coord.x += particles.at(i).pose.coord.x;
        mean.coord.y += particles.at(i).pose.coord.y;
        mean.angle = mean.angle.merge(particles.at(i).pose.angle, i, 1);
    }
    mean.coord.x = mean.coord.x/particles.size();
    mean.coord.y = mean.coord.y/particles.size();

    //find particle with smalest distance to the mean
    DirectedCoord position = particles.at(0).pose;
    float min_dist = mean.coord.dist(particles.at(0).pose.coord);

    //and calculate confidence with sum of squared errors
    float sum_of_squared_error = 0.0f;
    for (auto &particle: particles) {
        float tmp_dist = mean.coord.dist(particle.pose.coord);
        sum_of_squared_error += tmp_dist * tmp_dist;
        if (tmp_dist < min_dist) {
            position = DirectedCoord(particle.pose);
            min_dist = tmp_dist;
        }
    }
    float mean_dist = (sum_of_squared_error /particles.size());
    confidence = 1.0f;
    if (mean_dist >= 0.5f){
        confidence = 0.0f;
    } 
    if ((position.coord.dist(mean.coord) + fabs(position.angle.dist(mean.angle).rad))< 0.5) {
        pos = mean;
    }
    else{
        pos = position;
    }
}


//returns confidence of particle position
float ParticleFilter::adjustParticlesWithLandmarkHypos(const pair<vector<DirectedCoord>,int> &hypos){
    //find closest hypo
    float ADJUSTMENT_DIST = 1.5f;
    float minDist = ADJUSTMENT_DIST;
    float second_minDist = ADJUSTMENT_DIST;
    DirectedCoord minHypo;
    for (DirectedCoord h: hypos.first){
        for (auto particle :particles){
            float tmp_dist = h.coord.dist(particle.pose.coord)+fabs(h.angle.dist(particle.pose.angle).rad);
            if (tmp_dist < minDist){
                second_minDist = minDist;
                minDist = tmp_dist;
                minHypo = h;
            }
            else if (tmp_dist< second_minDist){
                second_minDist = tmp_dist;
            }
        }
    }
    //if one hypo close , else hypos to unreliable
    if ((minDist < ADJUSTMENT_DIST) and ((second_minDist- minDist) < 0.3)){
        float replacement_share; //higher replacement share for higher confidence
        int confidence = hypos.second;
        if (confidence == 1){ //1 landmark uses for hypothese
            replacement_share = 0.02;
        }
        else{ //min 2 landmarks used
            replacement_share = 0.05;
        }
        vector<DirectedCoord> position(1,minHypo);
        setParticlesToPosition(position, 0.1 , 0.1, 0.1, replacement_share);
    }
    return 0.0f;
}

/*take particles according to their weight:
generate a random number X between 0 an 1;
go trough particles an sum up their weights untill the sum is bigger then X
take the last particle you look at.
*/
void ParticleFilter::resample() {
    uniform_real_distribution<double> uniform_distribution(0.0, 1.0f);
    vector<Particle> tmp_particles = particles;
    for (uint i = 0; i < conf.numParticles; i++) {
        float beam = uniform_distribution(generator);
        size_t count_particle = 0;
        for (float sum_weight = particles.at(0).weight; 
                (sum_weight < beam) and (count_particle < (conf.numParticles-1));
                sum_weight += particles.at(count_particle).weight) {
            count_particle++;
        }
        tmp_particles.at(i).setParticle(DirectedCoord(particles.at(
                count_particle).pose.coord.x,
                                             particles.at(count_particle).pose.coord.y,
                                             particles.at(count_particle).pose.angle.rad),
                                             1.0f/conf.numParticles);
    }
    //copy the choosen particles from tmp to particles
    particles = move(tmp_particles);
}

/*
this is the main_fuction wich is called from "external"

it controlls the general process and calls the functions
    -move particels
    -measurement update
    -resample

*/
void ParticleFilter::update(const vector<VisionResult> &visionresults,
                                 DirectedCoord odometry, const pair<vector<DirectedCoord>,int> &hypos) {

    // omit for penalty goaly (for fps gain)
    if(conf.role == RobotRole::PENALTYGOALIE){
        return;
    }
    //sample particles from odometry / move particles according to odometry with spreading
    moveParticles(odometry);


    if (!visionresults.empty() and !isPenalized) {
        //weight particles with visionResults if particles weighted, normalize and resample
        if (measurementModel(visionresults)) {
            normalizeParticle();
            //take particles according to their weight
            lowVarianzeResample();
            //resample();
        }
    }
    calculatePose();
    adjustParticlesWithLandmarkHypos(hypos);
}


void ParticleFilter::setPosition(DirectedCoord pos) {
    for (uint i = 0; i < conf.numParticles; ++i) {
        particles.at(i).setParticle(pos, 1.0f/conf.numParticles);
    }
}


// return current position
DirectedCoord ParticleFilter::get_position(const float &step_pos,
        const float &step_rad) const {

    float _min_pos = (step_pos < step_bound) ? step_bound : step_pos;
    float _min_rad = (step_rad < step_bound) ? step_bound : step_rad;
    return DirectedCoord(round((1.0f / _min_pos) * pos.coord.x) * _min_pos,
                         round((1.0f / _min_pos) * pos.coord.y) * _min_pos,
                         round((1.0f / _min_rad) * pos.angle.rad) * _min_rad);
}

// return current position
float ParticleFilter::get_confidence() {

    if ((confidence>0.0f) and (confidence <=1.0f)) {
        return confidence;
    }
    return 0.0f;
}

vector<DirectedCoord> ParticleFilter::getHypothesesVector() {
    vector<DirectedCoord> ret;

    //for (size_t i = 0; i < conf.numParticles; ++i)
    for (auto it=particles.begin(); it!=particles.end(); ++it) {
        ret.push_back(it->pose);
    }

    return ret;
}


//------------------------------------------------------------------------------------------------------
//HELPER FUNCTIONS:
//------------------------------------------------------------------------------------------------------
void ParticleFilter::sortParticle() {
    auto lambda_sort = [](const Particle& p1, const Particle& p2)
                       -> bool { return p1.weight > p2.weight; };
    sort(particles.begin(), particles.end(), lambda_sort);
}

/*vector<tHypoWithWeight> ParticleFilter::getHypothesesVectorWithWeights() {
    vector<tHypoWithWeight> ret;
    for (auto it=particles.begin(); it!=particles.end(); ++it) {

        tHypoWithWeight t;
        t.push_back(it->pose.coord.x);
        t.push_back(it->pose.coord.y);
        t.push_back(it->pose.angle.rad);
        t.push_back(it->weight);
        ret.push_back(t);
    }
    return ret;
}*/

void ParticleFilter::normalizeParticle() {
    float sum_weights = 0.0f;
    for (size_t count_particle = 0; count_particle<particles.size();
            ++count_particle) {
        sum_weights += particles[count_particle].weight;
    }
    if (sum_weights > 0.0f) {
        for (size_t count_particle = 0; count_particle<particles.size();
                ++count_particle) {
            particles[count_particle].weight =
                (particles[count_particle].weight)/sum_weights;
        }
    } else {
        for (size_t count_particle = 0; count_particle<particles.size();
                ++count_particle) {
            particles[count_particle].weight = 1.0f/conf.numParticles;
        }

    }
}

constexpr float ParticleFilter::fastExp(float x) const {
     x = 1.f + x / 256.f;
     x *= x;
     x *= x;
     x *= x;
     return x;
}

const float ParticleFilter::prob(float x, float deviation) const {
    /*constexpr float mean = 0.0f;
    float inner = (x - mean) / deviation;
    inner *= inner;
    return 1.0f/(deviation*sqrt(2*M_PI_F))*fastExp(-0.5f*inner);*/
    float mean = 0.0f;
    return 1.0f/(deviation*std::sqrt(2*M_PI_F))*std::exp(-0.5f*powf(((x-mean)/deviation), 2));
}



vector<Feature> ParticleFilter::createLineFeature(const Particle particle) {
    vector<Feature> pfLines;
    for (auto &line: conf.pf->getLines()) {
        /// penaltymarks are still treaten as line in our playingfield...
        // and don't use short lines
        // penalty box back lines, seems to lead to error TODO
        if ((line.name == Line::OWN_PENALTY_SHOOTMARK)
                or (line.name == Line::OPP_PENALTY_SHOOTMARK)
                or (line.name == Line::OWN_GOALBOX_BACK)
                or (line.name == Line::OPP_GOALBOX_BACK)
                or (line.name == Line::OPP_PENALTY_LEFT)
                or (line.name == Line::OPP_PENALTY_RIGHT)
                or (line.name == Line::OWN_PENALTY_LEFT)
                or (line.name == Line::OWN_PENALTY_RIGHT)
                or (line.name == Line::OPP_GOALBOX_LEFT)
                or (line.name == Line::OPP_GOALBOX_RIGHT)
                or (line.name == Line::OWN_GOALBOX_LEFT)
                or (line.name == Line::OWN_GOALBOX_RIGHT)
                ) {
            continue;
        }
        //calculate distance and angle between particle and conf.pf landmark
        //and save the legth of the line
        Coord start(line.start_x, line.start_y);
        Coord end(line.end_x, line.end_y);
        DirectedCoord pointOnLine;
        pointOnLine.coord = particle.pose.coord.closestPointOnLine(start, end);
        Coord rcsline = pointOnLine.toRCS(particle.pose).coord;
        float dist = particle.pose.coord.dist(pointOnLine.coord);
        float angle = rcsline.angle().rad;
        float orientation = (end-start).direction() - particle.pose.angle.rad;

        pfLines.push_back(Feature(JSVISION_LINE, dist, angle, orientation,(int)line.name));
    }
    return pfLines;
}

vector<Feature> ParticleFilter::createGoalFeature(const Particle particle) {
    std::vector<Feature> pfGoals;
    for (auto &pole: conf.pf->getPoals()) {
        //calculate distance and angle between particle and conf.pf landmark
        float dist = particle.pose.coord.dist(Coord(pole.wcs_x, pole.wcs_y));
        Coord rcs(pole.wcs_x-particle.pose.coord.x, pole.wcs_y-particle.pose.coord.y);
        float angle = rcs.angle().rad;
        pfGoals.push_back(Feature(JSVISION_GOAL, dist, angle));
    }
    return pfGoals;
}

vector<Feature> ParticleFilter::createCircleFeature(const Particle particle) {
    vector<Feature> pfCircles;
    //calculate distance and angle between particle and conf.pf landmark
    DirectedCoord center(0.0f,0.0f,0.0f);
    Coord rcsCenter = center.toRCS(particle.pose).coord;
    float dist = rcsCenter.dist();
    float angle = rcsCenter.angle().rad;
    pfCircles.push_back(Feature(JSVISION_CIRCLE, dist, angle));
    return pfCircles;
}

vector<Feature> ParticleFilter::createLCrossFeature(const Particle particle) {
    vector<Feature> pfLCrosses;
    for (auto &cross: conf.pf->getLCrosses()) {
        //calculate distance and angle between particle and conf.pf landmark
        DirectedCoord crossCoord(cross.wcs_x, cross.wcs_y, cross.wcs_alpha);
        Coord rcsCross = crossCoord.toRCS(particle.pose).coord;
        float dist = rcsCross.dist();
        float angle = rcsCross.angle().rad;
        float orientation = Angle::normalize(cross.wcs_alpha) -
                            particle.pose.angle.rad;
        pfLCrosses.push_back(Feature(JSVISION_LCROSS, dist, angle, orientation));
    }
    return pfLCrosses;
}

vector<Feature> ParticleFilter::createTCrossFeature(const Particle particle) {
    vector<Feature> pfTCrosses;
    for (auto &cross: conf.pf->getTCrosses()) {
        //calculate distance and angle between particle and conf.pf landmark
        DirectedCoord crossCoord(cross.wcs_x, cross.wcs_y, cross.wcs_alpha);
        Coord rcsCross = crossCoord.toRCS(particle.pose).coord;
        float dist = rcsCross.dist();
        float angle = rcsCross.angle().rad;
        float orientation = Angle::normalize(cross.wcs_alpha) -
                            particle.pose.angle.rad;
        pfTCrosses.push_back(Feature(JSVISION_TCROSS, dist, angle, orientation));
    }
    return pfTCrosses;
}

vector<Feature> ParticleFilter::createXCrossFeature(const Particle particle) {
    vector<Feature> pfXCrosses;
    for (auto &cross: conf.pf->getXCrosses()) {
        //calculate distance and angle between particle and conf.pf landmark
        DirectedCoord crossCoord(cross.wcs_x, cross.wcs_y, cross.wcs_alpha);
        Coord rcsCross = crossCoord.toRCS(particle.pose).coord;
        float dist = rcsCross.dist();
        float angle = rcsCross.angle().rad;
        float orientation = Angle::normalize(cross.wcs_alpha) -
                            particle.pose.angle.rad;
        pfXCrosses.push_back(Feature(JSVISION_XCROSS, dist, angle, orientation));
    }
    return pfXCrosses;
}
