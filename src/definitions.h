#pragma once

#include <vector>

#include <coords.h>
#include <platform.h>

/**
 * basis class of all objects sent around between modules, contains messageType, sender id, timestamp.
*/
class Message {
public:
    /// default ctor, setting sender id of robot
    Message();

    TimestampMs age() const;
    TimestampMs timestamp; ///< when did this happen (see note on \ref timestamps)
};


/** robot position, see Message for details. */
class Robot: public Message {
public:
    /**
     * ctor for robot. Sets all fields to default values.
     */
    Robot();
    Robot(Coord pos);
    Robot(DirectedCoord pos);
    Robot(DirectedCoord pos, DirectedCoord GTpos);

    void setUnknownPose(); ///< sets all confidence and pose data to initial values

    // set robot from string
    bool setFromString(const std::string &data);

    int id; ///< id of the robot that has this pose (0..2 for our bots, else unknown)
    RobotRole role; ///< instance of robot role
    int fallen; ///< how long is the robot fallen down in seconds, -1 if not fallen.
    bool active; ///< is robot active, i.e. not penalized and so on

    DirectedCoord pos; ///< pos, see \ref coordsys
    float confidence; ///< confidence (see note on \ref confidence)

    DirectedCoord GTpos; ///< ground thruth position
    float GTconfidence; ///< ground thruth confidence
    TimestampMs GTtimestamp; ///< A Timestamp, @see getTimestampInMs()
};


std::ostream &operator<<(std::ostream &s,
                         const Robot &r); ///< stream output function

// vim: set ts=4 sw=4 sts=4 expandtab:
