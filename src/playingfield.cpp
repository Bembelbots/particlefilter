/* jsplayingfield.cpp
 * @author Christian Becker
 * Version : $Id$
 */

#include "playingfield.h"
#include <cmath>
#include <constants.h>
#include <cassert>
#include <algorithm>

using namespace std;

/*
 *   _lengthInsideBounds
 *   _penaltyLength
 *      -x           x
 * |------------------------|
 * |           |            | y
 * |--|        |         |--|
 * |  |        |         |  | _widthInsideBounds
 * |  |        |         |  | _penaltyWidth
 * |--|        |         |--|
 * |           |            | -y
 * |------------------------|
 *   own half     opposite
 */


PlayingField::PlayingField(FieldSize fieldSize) {
    _size = fieldSize;

    switch (_size) {
    case FieldSize::JRL:
        // This the JRL's permanent playingfield (defined accordning to pre 2012 SPL rules)
        createField({
                .fieldLength = 5.95f,
                .fieldWidth = 3.95f,
                .centerCircleDiameter = 1.15f,
                .penaltyCrossDistance = 1.35f
            });
        break;
    case FieldSize::HTWK:
        // field for HTWK Leipzig event, May 2014
        createField({
                .fieldLength = 7.5f,
                .fieldWidth = 5.f
            });
        break;
    case FieldSize::TINY:
        // tiny 4x1.8m field for exhibitions
        createField({
                .fieldLength = 4.f,
                .fieldWidth = 1.75f,
                .centerCircleDiameter = 0.8f,
                .penaltyCrossDistance = 1.1f,
                .penaltyAreaLength = 0.55f,
                .penaltyAreaWidth = 1.f,
                .goalWidth = 0.56f,
                .goalPostDiameter = 0.1f,
                .goalboxDepth = 0.3f,
            });

        break;
    case FieldSize::SPL2020:
        // This the JRL's permanent playingfield (defined accordning to pre 2012 SPL rules)
        createField({
                .fieldLength = 9.f,
                .fieldWidth = 6.f,
                .centerCircleDiameter = 1.5f,
                .penaltyCrossDistance = 1.4f,
                .penaltyAreaLength = 1.65f,
                .penaltyAreaWidth = 4.f
            });
        break;
    case FieldSize::SPL:
    default:
        // use function defaults defined in header for current SPL field,
        // according to the Rules 2013 Draft, published on November 8th 2012
        createField(FieldMeasurements());
    }

    assert(_poles.size() == PoleMAX);
    assert(_lines.size() == LineMAX);
    assert(_crosses.size() == CrossMAX);
}

PlayingField::~PlayingField() {
}

void PlayingField::createField(const FieldMeasurements &fm) {

    // total size of field (playing area + border strips)
    _length = fm.fieldLength + 2 * fm.borderStripWidth;
    _width = fm.fieldWidth + 2 * fm.borderStripWidth;

    // set length and width (of area inside the lines) in meters
    _lengthInsideBounds = fm.fieldLength;
    _widthInsideBounds = fm.fieldWidth;

    _lineWidth = fm.lineWidth;

    // set Goal width (distance between center of goal posts)
    _goalWidth = fm.goalWidth + fm.goalPostDiameter;

    _penaltyLength = fm.penaltyAreaLength;
    _penaltyWidth = fm.penaltyAreaWidth;

    // center circle
    _circle.wcs_x = 0.0f;
    _circle.wcs_y = 0.0f;
    _circle.name = Line::CENTER_CIRCLE;
    _circle.wcs_radius = fm.centerCircleDiameter / 2.f;

    _lines.clear();
    _crosses.clear();

    _penaltyCrossDistance = fm.penaltyCrossDistance;
    _penaltyCrossSize = fm.penaltyCrossSize;

    const float x = fm.fieldLength / 2.f;
    const float y = fm.fieldWidth / 2.f;

    // outer borders
    createNewLineDefinition(0.0f * M_PI_F,     x,     y,    -x,     y, Line::LEFT);
    createNewLineDefinition(0.0f * M_PI_F,     x,    -y,    -x,    -y,
                            Line::RIGHT);
    createNewLineDefinition(0.5f * M_PI_F,     x,     y,     x,    -y, Line::TOP);
    createNewLineDefinition(0.5f * M_PI_F,    -x,     y,    -x,    -y,
                            Line::BOTTOM);

    // center line
    createNewLineDefinition(0.5f * M_PI_F,  0.0f,     y,  0.0f,    -y,
                            Line::MIDDLE);

    const float px = x - fm.penaltyAreaLength;
    const float py = fm.penaltyAreaWidth / 2.f;
    const float mx = x - fm.penaltyCrossDistance;

    // opponent's penalty area
    createNewLineDefinition(0.5f * M_PI_F,  px,    py,  px,  -py,
                            Line::OPP_PENALTY);
    createNewLineDefinition(0.0f * M_PI_F,   x,    py,  px,   py,
                            Line::OPP_PENALTY_LEFT);
    createNewLineDefinition(0.0f * M_PI_F,   x,   -py,  px,  -py,
                            Line::OPP_PENALTY_RIGHT);
    createNewLineDefinition(0.0f * M_PI_F,  mx,  0.0f,  mx, 0.0f,
                            Line::OPP_PENALTY_SHOOTMARK);

    // our penalty area
    createNewLineDefinition(0.5f * M_PI_F, -px,    py, -px,  -py,
                            Line::OWN_PENALTY);
    createNewLineDefinition(0.0f * M_PI_F,  -x,    py, -px,   py,
                            Line::OWN_PENALTY_LEFT);
    createNewLineDefinition(0.0f * M_PI_F,  -x,   -py, -px,  -py,
                            Line::OWN_PENALTY_RIGHT);
    createNewLineDefinition(0.0f * M_PI_F, -mx,  0.0f, -mx, 0.0f,
                            Line::OWN_PENALTY_SHOOTMARK);

    // goalbox
    const float gbCorner_y = fm.goalWidth/2 + fm.goalPostDiameter/2;
    const float frontCorner_x = fm.fieldLength/2;
    const float gbBackCorner_x = frontCorner_x + fm.goalboxDepth;

    createNewLineDefinition(0, gbBackCorner_x, gbCorner_y, gbBackCorner_x, -gbCorner_y,
                            Line::OPP_GOALBOX_BACK);
    createNewLineDefinition(0, frontCorner_x, gbCorner_y, gbBackCorner_x, gbCorner_y,
                            Line::OPP_GOALBOX_LEFT);
    createNewLineDefinition(0, frontCorner_x, -gbCorner_y, gbBackCorner_x, -gbCorner_y,
                            Line::OPP_GOALBOX_RIGHT);
    createNewLineDefinition(0, -gbBackCorner_x, gbCorner_y, -gbBackCorner_x, -gbCorner_y,
                            Line::OWN_GOALBOX_BACK);
    createNewLineDefinition(0, -frontCorner_x, gbCorner_y, -gbBackCorner_x, gbCorner_y,
                            Line::OWN_GOALBOX_LEFT);
    createNewLineDefinition(0, -frontCorner_x, -gbCorner_y, -gbBackCorner_x, -gbCorner_y,
                            Line::OWN_GOALBOX_RIGHT);
    

    const float c = fm.centerCircleDiameter / 2.0f;

    // these are the cross definitions
    createNewCrossDefinition(x,    y, 1.0f * M_PI_F, 2, Cross::L_OPP_LEFT);
    createNewCrossDefinition(0.0f,    y, 1.5f * M_PI_F, 3, Cross::T_LEFT);
    createNewCrossDefinition(-x,    y, 1.5f * M_PI_F, 2, Cross::L_OWN_LEFT);
    createNewCrossDefinition(x,   -y, 0.5f * M_PI_F, 2, Cross::L_OPP_RIGHT);
    createNewCrossDefinition(0.0f,   -y, 0.5f * M_PI_F, 3, Cross::T_RIGHT);
    createNewCrossDefinition(-x,   -y, 0.0f * M_PI_F, 2, Cross::L_OWN_RIGHT);
    createNewCrossDefinition(0.0f,    c, 0.5f * M_PI_F, 4, Cross::X_LEFT);
    createNewCrossDefinition(0.0f,   -c, 0.5f * M_PI_F, 4, Cross::X_RIGHT);
    createNewCrossDefinition(x,   py, 1.0f * M_PI_F, 3,
                             Cross::T_OPP_GOAL_LEFT);
    createNewCrossDefinition(x,  -py, 1.0f * M_PI_F, 3,
                             Cross::T_OPP_GOAL_RIGHT);
    createNewCrossDefinition(px,   py, 1.5f * M_PI_F, 2,
                             Cross::L_OPP_GOAL_LEFT);
    createNewCrossDefinition(px,  -py, 0.0f * M_PI_F, 2,
                             Cross::L_OPP_GOAL_RIGHT);

    createNewCrossDefinition(gbBackCorner_x, gbCorner_y, M_PI_F, 2,
                             Cross::L_OPP_GOALBOX_LEFT);
    createNewCrossDefinition(gbBackCorner_x, -gbCorner_y, 0.5f*M_PI_F, 2,
                             Cross::L_OPP_GOALBOX_RIGHT);
    createNewCrossDefinition(x,  gbCorner_y, 0.0f * M_PI_F, 3,
                             Cross::T_OPP_GOALBOX_LEFT);
    createNewCrossDefinition(x,  -gbCorner_y, 0.0f * M_PI_F, 3,
                             Cross::T_OPP_GOALBOX_RIGHT);


    createNewCrossDefinition(-px,   py, 1.0f * M_PI_F, 2,
                             Cross::L_OWN_GOAL_LEFT);
    createNewCrossDefinition(-px,  -py, 0.5f * M_PI_F, 2,
                             Cross::L_OWN_GOAL_RIGHT);

    createNewCrossDefinition(-gbBackCorner_x, gbCorner_y, 1.5f*M_PI_F, 2,
                             Cross::L_OWN_GOALBOX_LEFT);
    createNewCrossDefinition(-gbBackCorner_x, -gbCorner_y, 0, 2,
                             Cross::L_OWN_GOALBOX_RIGHT);

    createNewCrossDefinition(-x,  py, 0.0f * M_PI_F, 3,
                             Cross::T_OWN_GOAL_LEFT);
    createNewCrossDefinition(-x,  -py, 0.0f * M_PI_F, 3,
                             Cross::T_OWN_GOAL_RIGHT);

    createNewCrossDefinition(-x,  gbCorner_y, 1.0f * M_PI_F, 3,
                             Cross::T_OWN_GOALBOX_LEFT);
    createNewCrossDefinition(-x,  -gbCorner_y, 1.0f * M_PI_F, 3,
                             Cross::T_OWN_GOALBOX_RIGHT);

    // poles
    const float g = (fm.goalWidth + fm.goalPostDiameter) / 2;
    createNewPoleDefinition(x,  g, fm.goalPostHeight, fm.goalPostDiameter, 0,
                            Pole::OPP_LEFT);
    createNewPoleDefinition(x, -g, fm.goalPostHeight, fm.goalPostDiameter, 0,
                            Pole::OPP_RIGHT);
    createNewPoleDefinition(-x,  g, fm.goalPostHeight, fm.goalPostDiameter, 0,
                            Pole::OWN_LEFT);
    createNewPoleDefinition(-x, -g, fm.goalPostHeight, fm.goalPostDiameter, 0,
                            Pole::OWN_RIGHT);



    // goals
    createNewGoalDefinition(_poles[static_cast<int>(Pole::OPP_LEFT)],
                            _poles[static_cast<int>(Pole::OPP_RIGHT)],
                            fm.goalWidth + fm.goalPostDiameter, 1);
    createNewGoalDefinition(_poles[static_cast<int>(Pole::OWN_LEFT)],
                            _poles[static_cast<int>(Pole::OWN_RIGHT)],
                            fm.goalWidth + fm.goalPostDiameter, 0);

    _outerDiagonal = sqrtf(_length * _length + _width * _width);
    _innerDiagonal = sqrtf(_lengthInsideBounds * _lengthInsideBounds +
                           _widthInsideBounds * _widthInsideBounds);

    // according to Tim's transformation:
    // folgt aus den Verhältnis, dass der Kreis durch die Torpfosten 
    // und den mittelpunkt des Strafraums gehen soll
    float centerpointGoaliDefenderCircle_xdiff = (_penaltyWidth/2.0f)*(_penaltyWidth/2.0f)/(_penaltyLength)
                                    -((_penaltyLength)/4);
    centerpointGoaliDefenderCircle = {-(_lengthInsideBounds/2+centerpointGoaliDefenderCircle_xdiff),0.0f};
    radiusGoaliDefenderCircle = (_penaltyLength/2.0f)+ centerpointGoaliDefenderCircle_xdiff;
}


DirectedCoord PlayingField::getManualSetPose(int botId,
        bool hasKickoff) const {
    DirectedCoord p;

    // always look towards enemy
    p.angle.set(0.0f);

    switch (_size) {
    case FieldSize::SPL:
        switch (botId) {
        case 0:
            p.coord = {-4.5f, 0.0f};
            break;
        case 1:
            if (hasKickoff)
                p.coord = {-0.7f, 0.0f};
            else
                p.coord = {-3.6f, 0.5f};
            break;
        case 2:
            if (hasKickoff)
                p.coord = {-1.9f, -0.8f};
            else
                p.coord = {-3.6f, 0.5f};
            break;
        case 3:
            if (hasKickoff)
                p.coord = {-3.6f, 1.1f};
            else
                p.coord = {-3.6f, -2.0f};
            break;
        case 4:
            if (hasKickoff)
                p.coord = {-3.6f, -1.1f};
            else
                p.coord = {-3.6f, 2.0f};
            break;
        default:
            p.coord = {0.0f, 0.0f};
            break;
        }
        break;
    default:
        switch (botId) {
        case 0:
            p.coord = {-2.9f, 0.0f};
            break;
        case 1:
            if (hasKickoff)
                p.coord = {-0.7f, 0.0f};
            else
                p.coord = {-2.3f, 0.7f};
            break;
        case 2:
            if (hasKickoff)
                p.coord = {-1.2f, 1.1f};
            else
                p.coord = {-2.3f, -1.55f};
            break;
        case 3:
            if (hasKickoff)
                p.coord = {-2.3f, -1.1f};
            else
                p.coord = {-2.3f, 1.55f};
            break;
        default:
            p.coord = {0.0f, 0.0f};
            break;
        }
    }
    return p;
}

Coord PlayingField::getPlayingBoxStart(RobotRole role) {
    Coord p;
    switch (role) {
    case RobotRole::STRIKER:
        p = { _lengthInsideBounds/6.0f,  _widthInsideBounds/2.0f};
        break;
    case RobotRole::DEFENDER:
        p = {-_lengthInsideBounds/2.0f, -_widthInsideBounds/2.0f};
        break;
    case RobotRole::SUPPORTER_DEFENSE:
        p = {-_lengthInsideBounds/4.0f, -_widthInsideBounds/4.0f};
        break;
    default:
        p = {-_lengthInsideBounds/2.0f, -_widthInsideBounds/2.0f};
        break;
    }
    return p;
}

Coord PlayingField::getPlayinBoxEnd(RobotRole role) {
    Coord p;
    switch (role) {
    case RobotRole::STRIKER:
        p = { _lengthInsideBounds/2.0f, _widthInsideBounds/2.0f};
        break;
    case RobotRole::DEFENDER:
        p = {-_lengthInsideBounds/6.0f, _widthInsideBounds/2.0f};
        break;
    case RobotRole::SUPPORTER_DEFENSE:
        p = { _lengthInsideBounds/4.0f, _widthInsideBounds/4.0f};
        break;
    default:
        p = { _lengthInsideBounds/2.0f, _widthInsideBounds/2.0f};
        break;
    }
    return p;
}


Coord PlayingField::getPenaltyMarkPosition(bool opponent) const {
    const auto name = (opponent) ? Line::OPP_PENALTY_SHOOTMARK :
                                    Line::OWN_PENALTY_SHOOTMARK;
    const auto &l = _lines[int(name)];
    return {l.start_x, l.start_y};
}


DirectedCoord PlayingField::getReadyPose(int botId, bool hasKickoff) const {
    DirectedCoord p;
    assert(0 <= botId && botId <= 4);
    switch (botId) {
        case 0:
            p.coord = { -_lengthInsideBounds/2.0f + 0.1f , 0.f};
            break;
        case 1:
            // TODO: switch Y coordinate for Team-Team
            p.coord = { -_lengthInsideBounds/2.0f + _penaltyCrossDistance,
                        _goalWidth/ 4.f
                      };
            break;
        case 2:
            p.coord = { -_lengthInsideBounds/2.0f + _penaltyCrossDistance + 0.5f,
                        -_goalWidth/ 4.f
                      };
            break;
        case 3:
            p.coord = { -2 * _circle.wcs_radius ,
                        _goalWidth/ 2.f
                      };
            break;
        case 4:
            if (hasKickoff)
                p.coord = {-_circle.wcs_radius,  0.f};
            else
                p.coord = {-_circle.wcs_radius - 0.60f, 0.f};
            break;
    }
    return p;
}
/*
 *   _lengthInsideBounds
 *   _penaltyLength
 *      -x           x
 * |------------------------|
 * |           |            | y
 * |--|        |         |--|
 * |  |        |         |  | _widthInsideBounds
 * |  |        |         |  | _penaltyWidth
 * |--|        |         |--|
 * |           |            | -y
 * |------------------------|
 *   own half     opposite
 *
 */

bool PlayingField::insidePenaltybox(const Coord &pos) const {
    float errorTolerance = 0.1f;
    // width = from goal to center cirlce, 60cm for example (x)
    // length = from pole to pole, 2.2m for example (y)
    float maxY = _penaltyWidth / 2.0f + errorTolerance;
    bool widthOk = (pos.y >= -maxY) && (pos.y <= maxY);

    float minX = -((_lengthInsideBounds / 2.0) + errorTolerance);
    float maxX = minX + _penaltyLength;
    bool heightOk = (pos.x >= minX) && (pos.x <= maxX);

    return widthOk && heightOk;
}

// returns default unpenalize pose!
vector<DirectedCoord> PlayingField::getUnpenalizedPose() const {
    const float l = _lengthInsideBounds/2.f - _penaltyCrossDistance;
    const float w = _widthInsideBounds/2.f;
    static const float pi2 = M_PI_F / 2.f;
    return {{-l, w, -pi2}, {-l, -w, pi2}};
}


// returns possible manual placement pose!
vector<DirectedCoord> PlayingField::getManualPlacementPose(bool kickoff, bool isGoali) const {
    if (isGoali){
        float x = -_lengthInsideBounds/2.f ;
        float y = 0.f;
        float alpha = 0.f;
        return {{x,y,alpha}};
    }
    if (!kickoff){
        DirectedCoord pos_id0(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              _widthInsideBounds/2.f*0.25f,  0.0f);
        DirectedCoord pos_id1(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              _widthInsideBounds/2.f*0.7f, 0.0f);
        DirectedCoord pos_id2(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              -_widthInsideBounds/2.f*0.25f,  0.0f);
        DirectedCoord pos_id3(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              -_widthInsideBounds/2.f*0.7f,  0.0f);
        return {pos_id0, pos_id1, pos_id2, pos_id3};
    }else{
        DirectedCoord pos_id0(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              _widthInsideBounds/2.f*0.2f,  0.0f);
        DirectedCoord pos_id1(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              _widthInsideBounds/2.f*0.7f, 0.0f);
        DirectedCoord pos_id2(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              -_widthInsideBounds/2.f*0.25f,  0.0f);
        DirectedCoord pos_id3(-(_lengthInsideBounds/2.f)+ _penaltyLength+0.25f,
                              -_widthInsideBounds/2.f*0.7f,  0.0f);
        DirectedCoord pos_id4(-_circle.wcs_radius-0.3f,
                              0.0f,  0.0f);
        return {pos_id0, pos_id1, pos_id2, pos_id3, pos_id4};
    }
}

// returns initial Positions vektor[x] = Position id x
vector<DirectedCoord> PlayingField::getInitialPose() const {
    DirectedCoord pos_id0(-(_lengthInsideBounds/2.f) *0.75f,
                          -_widthInsideBounds/2.f,  M_PI_F/2.f);
    DirectedCoord pos_id1(-(_lengthInsideBounds/2.f) *0.75f,
                          _widthInsideBounds/2.f, -M_PI_F/2.f);
    DirectedCoord pos_id2(-(_lengthInsideBounds/2.f) *0.5f,
                          -_widthInsideBounds/2.f,  M_PI_F/2.f);
    DirectedCoord pos_id3(-(_lengthInsideBounds/2.f) *0.3f,
                          _widthInsideBounds/2.f, -M_PI_F/2.f);
    DirectedCoord pos_id4(-(_lengthInsideBounds/2.f) *0.3f,
                          -_widthInsideBounds/2.f,  M_PI_F/2.f);
    DirectedCoord pos_id5(-(_lengthInsideBounds/2.f) *0.5f,
                          _widthInsideBounds/2.f,  -M_PI_F/2.f);
    return {pos_id0, pos_id1, pos_id2, pos_id3, pos_id4, pos_id5};
}

float PlayingField::penaltyBoxToFieldBorderLength() const {
    return (_width - _penaltyWidth) / 2;
}

float PlayingField::penaltyBoxToMiddleLineLength() const {
    return (_length / 2) - _penaltyLength;
}

// creating functions used in constructor to fill the objekt with live

void PlayingField::createNewLineDefinition(const float alpha,
        const float startX, const float startY, const float endX, const float endY,
        const Line lineType) {
    assert(_lines.size() == static_cast<size_t>(lineType));
    LandmarkLine line;

    line.equationNormal = vectorCrossProductUnNormed({startX, startY, 1.f}, {endX, endY, 1.f});
    line.equation = getHesseNormalFormOfLine(line.equationNormal);

    /* line.wcs_alpha = alpha; */

    line.start_x = startX;
    line.start_y = startY;
    line.end_x = endX;
    line.end_y = endY;

    line.name = lineType;

    _lines.push_back(line);
}

void PlayingField::createNewCrossDefinition(const float x, const float y,
        const float alpha, const int degree,
        const Cross crossType) {
    //std::cout << _crosses.size() << ","<< static_cast<size_t>(crossType)<<std::endl;
    assert(_crosses.size() == static_cast<size_t>(crossType));
    LandmarkCross cross;

    cross.equation = {x, y, 1.f};
    cross.wcs_x = x;
    cross.wcs_y = y;
    cross.wcs_alpha = alpha;
    cross.degree = degree;
    cross.name = crossType;

    _crosses.push_back(cross);
}

LandmarkPole PlayingField::createNewPoleDefinition(const float x,
        const float y,
        const float height,
        const float width,
        const int color,
        const Pole poleType) {
    assert(_poles.size() == static_cast<size_t>(poleType));

    LandmarkPole pole;
    pole.wcs_x = x;
    pole.wcs_y = y;
    pole.wcs_width = width;
    pole.wcs_height = height;
    pole.color = color;
    pole.name = poleType;
    pole.equation = {x, y, 1.f};

    _poles.push_back(pole);

    return pole;
}

void PlayingField::createNewGoalDefinition(const LandmarkPole &left,
        const LandmarkPole &right,
        const float poleDistance,
        bool opponent) {
    LandmarkGoal goal;
    goal.left = left;
    goal.right = right;
    goal.wcs_pole_distance = poleDistance;
    goal.color = left.color;

    if (opponent) {
        _goals.first = goal;
    } else {
        _goals.second = goal;
    }
}




void PlayingField::createDistVecOfCrosses() {
    for (size_t i = 0; i < CrossMAX; i++) {
        _crosses.at(i).distances.clear();

        for (size_t j = 0; j <  CrossMAX; j++) {
            float dist = Coord(_crosses.at(i).equation).dist(_crosses.at(j).equation);
            _crosses.at(i).distances.push_back(dist);
        }
        assert(_crosses.at(i).distances.size() == CrossMAX);
    }
}

// geting functions ... to get specific objects

vector<LandmarkLine> PlayingField::getLines() const {
    return _lines;
}


// get Crosses/Edges of a given degree
vector<LandmarkCross> PlayingField::getCrosses(const int &degree) const {
    vector<LandmarkCross> result;
    std::copy_if(_crosses.begin(), _crosses.end(), std::back_inserter(result),
    [&](const LandmarkCross &c) {
        return c.degree == degree;
    });
    return result;
}


vector<LandmarkCross> PlayingField::getLCrosses() const {
    return getCrosses(2);
}

vector<LandmarkCross> PlayingField::getTCrosses() const {
    return getCrosses(3);
}

vector<LandmarkCross> PlayingField::getXCrosses() const {
    return getCrosses(4);
}

LandmarkCross  PlayingField::getCross(const Cross name) const {
    int i = static_cast<int>(name);
    // check whether someone is doing something nasty
    assert(i > 0);
    assert(i < static_cast<int>(_crosses.size()));

    // array indices are equal to enum numbers (guaranteed by assert)
    return _crosses[i];
}


// get all crosses
vector<LandmarkCross> PlayingField::getAllCrosses() const {
    vector <LandmarkCross> result;
    vector <LandmarkCross> deg2;
    vector <LandmarkCross> deg3;
    vector <LandmarkCross> deg4;

    deg2 = getTCrosses();
    deg3 = getLCrosses();
    deg4 = getXCrosses();

    result.insert(result.end(), deg2.begin(), deg2.end());
    result.insert(result.end(), deg3.begin(), deg3.end());
    result.insert(result.end(), deg4.begin(), deg4.end());

    return result;
}

std::vector<LandmarkPole> PlayingField::getPoals() const {
    return _poles;
}

vector<LandmarkOrientation> PlayingField::getDistsAndAngles(
    const DirectedCoord &from, int type) {

    vector<DirectedCoord> landmark_positions;

    // len(L)     = 8, type 0
    // len(T)     = 6, type 1
    // len(X)     = 2, type 2
    // len(poles) = 4, type 3

    vector<LandmarkOrientation> ret;

    // Crosses
    for (const auto &c: _crosses) {
        LandmarkOrientation lo;
        memset(&lo, 0, sizeof(lo));
        if (c.degree == 3) {
            lo.landmark = 1;
        } else if (c.degree == 4) {
            lo.landmark = 2;
        } else {
            lo.landmark = 0;
        }

        landmark_positions.emplace_back(c.wcs_x, c.wcs_y, 0.0f);
        ret.push_back(lo);
    }

    // Poles
    LandmarkOrientation lo;
    memset(&lo, 0, sizeof(lo));
    lo.landmark = 3;
    for (const auto &p: _poles) {
        landmark_positions.emplace_back(p.wcs_x, p.wcs_y, 0.0f);
        ret.push_back(lo);
    }

    auto rt = ret.begin();
    for (const auto &lp: landmark_positions) {
        // translate this target wcs position to rcs, according to the
        // own position in the wcs
        DirectedCoord guess = lp.toRCS(from);
        rt->rcs_dist = guess.coord.dist();
        rt->rcs_angle = guess.coord.angle().rad;
    }

    return ret;
}

DirectedCoord PlayingField::kick_off_position(bool has_kickoff){
        if (!has_kickoff){
            return DirectedCoord((-1.7f * _circle.wcs_radius), 0.0f, 0.0f);
        }
        return DirectedCoord(0.0f, 0.0f, 0.0f);
}



bool PlayingField::ball_outside_centercircle(Coord ball){
    float distance_from_center = sqrt(ball.x*ball.x + ball.y*ball.y);
    return (distance_from_center > _circle.wcs_radius);
}


DirectedCoord PlayingField::goaliDefenderPosition(Angle theta){
    DirectedCoord directedcenterpointCircle = {centerpointGoaliDefenderCircle,theta};
    return directedcenterpointCircle.walk(DirectedCoord(radiusGoaliDefenderCircle,0.0f,0.0f));
}

Angle PlayingField::getCurrentThetaGoali(DirectedCoord bot_pos){
    return  centerpointGoaliDefenderCircle.lookAt(bot_pos.coord).angle;

}



// vim: set ts=4 sw=4 sts=4 expandtab:
