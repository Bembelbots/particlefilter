/**
 * @author Module owner: Bembelbots Frankfurt, sinaditzel
 *
 *
 */
#pragma once

#include <random>
#include <map>
#include <vector>
#include "playingfield.h"
#include <visiondefinitions.h>
#include <coords.h>
#include <functional>
#include <cassert>


class Particle {
public:
    Particle(const DirectedCoord &data, const float &w);
    void setParticle(const DirectedCoord &data, const float &w);
    float weight;
    DirectedCoord pose;
};

class Feature {
public:
    Feature();
    Feature(const int type, const float dist, const float angle,
            const float orientation, int id);
    int type;
    float dist;
    float angle;
    //only for crosses
    float orientation;
    int id;
};


class ParticleFilter  {
public:

    class Settings {
    public:
        Settings(PlayingField *pf);
        PlayingField *pf;

        /*
         * number of particles to use.
         * more particles lead to better results,
         * but increase the runtime of the filter.
         */
        size_t numParticles;
        /*
         * error of odometry
         */
        DirectedCoord odoStdev;
        /*
         * start position of the mcs Koordinate System(Odometry)
         */
        DirectedCoord startMcsPosition;
        /*
         *  start position of the robot (WCS)...
         */
        DirectedCoord startPosition;

        int robot_id;
        bool has_kickoff;
        RobotRole role;
    };

    ParticleFilter(const Settings &conf);
    ~ParticleFilter();

    //EVENT HANDELING:
    // various events a localization-realization may react to
    typedef enum {
        EV_INTIAL, //0
        EV_LOST_GROUND,
        EV_GOT_GROUND,
        EV_BACK_UP,
        EV_FALLEN,
        EV_PENALIZED, //5
        EV_UNPENALIZED,
        EV_STATE_INITIAL,
        EV_STATE_READY,
        EV_STATE_SET,
        EV_STATE_PLAYING, //10
        EV_STATE_FINISHED
        } tLocalizationEvent;

    // handle (localization) events callback register here...
    void emit_event(const tLocalizationEvent &ev);
    void handle_event(const tLocalizationEvent &ev, std::function<void()> handler);
    
    // the event handlers
    std::map<tLocalizationEvent, std::function<void()>> ev_callbacks;

    void update(const std::vector<VisionResult> &visionresult,
                DirectedCoord odometry,
                const std::pair<std::vector<DirectedCoord>,int> &hypos);

    DirectedCoord get_position(
                const float &min_step_pos = 0.01f,
                const float &min_step_rad = 0.008f) const;

    float get_confidence();

    std::vector<DirectedCoord> getHypothesesVector();
    
    // set all particles to position "pos"
    void setPosition(DirectedCoord pos);


    //private:
    Settings conf;

    // my current position
    DirectedCoord pos;

    //confidence of position
    float confidence;

    //particles
    std::vector<Particle> particles;

    // needed to caculate random numbers
    std::default_random_engine generator;

    // last position of the mcs
    DirectedCoord lastMcsPosition;
    
    //saves if robot is fallen
    bool isFallenRobot;
    
    //after penalized or manual placement == true
    bool isReplaced;
    
    //is robot currently penalized
    bool isPenalized;

    //what was the gamestate robot has been penalized?
    GameState penalizedGamestate;
    
    //current gamestate
    GameState gamestate;

    std::vector<Feature> matchedLandmarks;

    // setting get_pos() granularity below this means: get the raw position values
    const float step_bound= 0.0001f;


    /*
     *handel loca events 
     */
    void initHandler();
    void penalizedHandler();
    void unpenalizedHandler();
    void playHandler();
    void readyHandler();
    void penaltyKickHandler();
    void setHandler();
    void fallenRobotHandler();
    void standUpHandler();
    void manualPlacementHandler();
    void globalLocalisation();

    std::pair<float,Feature> calculateProbabilityOfMatchingLandmark(Feature visionresult,
            std::vector<Feature> pf_landmarks);
    bool measurementModel(const std::vector<VisionResult> &vrs);
    void moveParticles(const DirectedCoord &odo);
    void lowVarianzeResample();
    void calculatePose();
    float adjustParticlesWithLandmarkHypos(const std::pair<std::vector<DirectedCoord>,int> &hypos);
 

    //helper:
    void setParticlesToPosition(std::vector<DirectedCoord> positions, float deviationX = 0.05f,
                                float deviationY = 0.05f, float deviationAlpha = 0.05f,
                                float amount = 1);
    std::vector<Feature> createLineFeature(const Particle particle);
    std::vector<Feature> createGoalFeature(const Particle particle);
    std::vector<Feature> createLCrossFeature(const Particle particle);
    std::vector<Feature> createTCrossFeature(const Particle particle);
    std::vector<Feature> createXCrossFeature(const Particle particle);
    std::vector<Feature> createCircleFeature(const Particle particle);
    void sortParticle();
    void normalizeParticle();
    constexpr inline float fastExp(float x) const;
    const inline float prob(float x, float dev = 0.8f) const;

    //unused
    void resample();
    void calculatePose2();
};



// vim: set ts=4 sw=4 sts=4 expandtab:
