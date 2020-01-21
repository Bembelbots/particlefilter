#include <sstream>

#include "definitions.h"

Robot::Robot() :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
}

Robot::Robot(Coord pos) :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
    this->pos = DirectedCoord();
    this->pos.coord = pos;
}

Robot::Robot(DirectedCoord pos) :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
    this->pos = pos;
}
Robot::Robot(DirectedCoord pos, DirectedCoord GTpos) :
    Message(),
    id(-1),
    role(RobotRole::STRIKER),
    fallen(-1),
    active(false),
    confidence(0),
    GTconfidence(0),
    GTtimestamp(-1) {
    this->pos = pos;
    this->GTpos = GTpos;
}

void Robot::setUnknownPose() {
    pos = DirectedCoord();
    confidence = 0.0f;
}

bool Robot::setFromString(const std::string &data) {
    int pos1, pos2;

    pos1 = data.find("id=");
    id = atoi(data.substr(pos1+3, 1).c_str());

    pos1 = data.find("t=")+2;
    pos2 = data.find(" c=");
    timestamp = atoi(data.substr(pos1, pos2-pos1).c_str());

    pos1 = data.find(" c=")+3;
    pos2 = data.find(" @ ");
    confidence = atof(data.substr(pos1, pos2-pos1).c_str());

    pos1 = data.find(" @ ")+3;
    pos2 = data.find("rad ");
    std::string p = data.substr(pos1, pos2-pos1);
    pos1 = p.find(", ");
    pos.coord.x = atof(p.substr(0, pos1).c_str());
    pos2 = p.find(", ", pos1+2);
    pos.coord.y = atof(p.substr(pos1+2, pos2-pos1-2).c_str());
    pos.angle = static_cast<float>(atof(p.substr(pos2+2,
                                        p.size()-pos2-2).c_str()));

    pos1 = data.find("RUTH: ")+6;
    pos2 = data.find(",Conf");
    std::string gt = data.substr(pos1, pos2-pos1);
    pos1 = gt.find(",");
    pos2 = gt.find(",", pos1+1);
    GTpos.coord.x = atof(gt.substr(0, pos1).c_str());
    GTpos.coord.y = atof(gt.substr(pos1+1, pos2-pos1-1).c_str());
    GTpos.angle = static_cast<float>(atof(gt.substr(pos2+1,
                                          p.size()-pos2-1).c_str()));

    pos1 = data.find("Conf: ")+6;
    GTconfidence = atof(data.substr(pos1, data.size() - pos1).c_str());

    return true;
}

Message::Message() :
    timestamp(getTimestampMs()) {
}

TimestampMs Message::age() const {
    if (timestamp < 0) {
        return 1000*1000;    //getTimestampMs();
    }
    return getTimestampMs() - timestamp;
}

std::ostream &operator<<(std::ostream &s, const Robot &r) {
    s << "Robot id=" << r.id << " t=" << r.timestamp << " c=" << r.confidence <<
      " @ " << r.pos.coord.x << ", " << r.pos.coord.y << ", " << r.pos.angle << "rad"
      << " GROUNDTRUTH: " << r.GTpos.coord.x << "," << r.GTpos.coord.y << "," <<
      r.GTpos.angle << ",Conf: " << r.GTconfidence << ", act=" << r.active;
    return s;
}


// vim: set ts=4 sw=4 sts=4 expandtab:
