/*
** jsplayingfield.h
 * @author Christian Becker
 *
 * Version : $Id$
**/

#pragma once

#include <types.h>
#include <mathtoolbox.h>
#include <vector>
#include <cstring>

/* This enums are used in jslinematching, so do not change them, if do not know what you do! */
enum class Line {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    MIDDLE,
    OPP_PENALTY,
    OPP_PENALTY_LEFT,
    OPP_PENALTY_RIGHT,
    OPP_PENALTY_SHOOTMARK,
    OWN_PENALTY,
    OWN_PENALTY_LEFT,
    OWN_PENALTY_RIGHT,
    OWN_PENALTY_SHOOTMARK,
    OPP_GOALBOX_BACK,
    OPP_GOALBOX_LEFT,
    OPP_GOALBOX_RIGHT,
    OWN_GOALBOX_BACK,
    OWN_GOALBOX_LEFT,
    OWN_GOALBOX_RIGHT,
    CENTER_CIRCLE,
    UNKNOWN
};
static const size_t LineMAX = static_cast<size_t>(Line::CENTER_CIRCLE);


/* This are the Crose names, do not change, theire are used in jslinemathing */
/* TODO: take names that are intuitive ... like it has been done with the lines */
/* The Crosses are seen from the center point of the field, facing the enemy goal*/
/*
 *      -x    Left        x
 * |------------------------|
 * |           |            | y
 * |--         |          --|
 * |  |        |         |  |
 * |  |        x-->      |  | Top
 * |  |        |         |  |
 * |--         |          --| -y
 * |           |            |
 * |------------------------|
 *  own half  Right   opposite
 */


enum class Cross {
    L_OPP_LEFT,
    T_LEFT,
    L_OWN_LEFT,
    L_OPP_RIGHT,
    T_RIGHT,
    L_OWN_RIGHT,
    X_LEFT,
    X_RIGHT,
    T_OPP_GOAL_LEFT,
    T_OPP_GOAL_RIGHT,
    L_OPP_GOAL_LEFT,
    L_OPP_GOAL_RIGHT,
    L_OPP_GOALBOX_LEFT,
    L_OPP_GOALBOX_RIGHT,
    T_OPP_GOALBOX_LEFT,
    T_OPP_GOALBOX_RIGHT,
    L_OWN_GOAL_LEFT,
    L_OWN_GOAL_RIGHT,
    L_OWN_GOALBOX_LEFT,
    L_OWN_GOALBOX_RIGHT,
    T_OWN_GOAL_LEFT,
    T_OWN_GOAL_RIGHT,
    T_OWN_GOALBOX_LEFT,
    T_OWN_GOALBOX_RIGHT
};
static const size_t CrossMAX = static_cast<size_t>(Cross::T_OWN_GOALBOX_RIGHT) +
                               1;


/* also used in linemathing */
enum class Pole {
    OPP_LEFT,
    OPP_RIGHT,
    OWN_LEFT,
    OWN_RIGHT
};
static const size_t PoleMAX = static_cast<size_t>(Pole::OWN_RIGHT) + 1;

/* given a wcs position, this calculates
 * a distance, angle and type to a Landmark,
 * in RCS data
 * this is usefull to match a vision result
 * to a particle for instance.
 */
struct LandmarkOrientation {
    float rcs_dist;
    float rcs_angle;
    // enum with type of landmark
    // 0 = crosses, type 2 (L)
    // 1 = crosses, type 3 (T)
    // 2 = crosses, type 4 (X)
    // 3 = poles
    int landmark;
};

struct LandmarkCross {
    Cross name; // enum "NAME" of the cross
    float wcs_x; // wcs pose in m
    float wcs_y;
    float wcs_alpha; // wcs angle in rad
    std::vector<float> equation; // wcs pose in homogenus vector form (x,y,1)
    int degree; /// 2=L,3=T,4=X
    std::vector<float>
    distances; // in this vector all distances to other crosses are stored, the index of the vector is equal to the enum name
};

struct LandmarkLine {
    Line name; // enum see above
    float start_x; // startpoint in wcs, m
    float start_y;
    float end_x; // endpoint
    float end_y;
    /* float wcs_alpha; // rad */
    std::vector<float>
    equation; /// line equation as vector of (a,b,c) from the lineequation a*x+b*y+c=0 in hnf!
    std::vector<float>
    equationNormal; /// line equation as vector of (a,b,c) from the lineequation a*x+b*y+c=0
};


struct LandmarkPole {
    Pole name;
    float wcs_x; /// m, wcs position
    float wcs_y;
    float wcs_width; // m
    float wcs_height;
    std::vector<float> equation; // wcs pose in homogenius vector form (x,y,1)
    int color; // should be same enum as from jsvision ...
};

struct LandmarkGoal {
    LandmarkPole  left;
    LandmarkPole  right;
    float wcs_pole_distance; // wcs distance of goal poles in m
    int color; // should be same enum as from jsvision ...
};

struct LandmarkCenterCircle {
    Line name; // the name is of type line because linematching uses it like this
    float wcs_x; // m
    float wcs_y;
    float wcs_radius;
};

class PlayingField {
public:
    PlayingField(FieldSize fieldSize);
    ~PlayingField();

    std::vector<LandmarkCross> getCrosses(const int &degree) const;
    LandmarkCross getCross(const Cross name) const;
    std::vector<LandmarkLine> getLines() const;
    std::vector<LandmarkCross> getLCrosses() const;
    std::vector<LandmarkCross> getTCrosses() const;
    std::vector<LandmarkCross> getXCrosses() const;
    std::vector<LandmarkCross> getAllCrosses() const;
    std::vector<LandmarkPole> getPoals() const;

    std::pair<LandmarkGoal, LandmarkGoal> _goals;
    std::vector<LandmarkPole> _poles;
    std::vector<LandmarkLine> _lines;
    std::vector<LandmarkCross> _crosses;
    LandmarkCenterCircle _circle;
    float _length;
    float _width;
    float _lengthInsideBounds;
    float _widthInsideBounds;
    float _penaltyWidth;
    float _penaltyLength;
    float _goalWidth;
    float _lineWidth;
    float _outerDiagonal;
    float _innerDiagonal;
    float _penaltyCrossDistance;
    float _penaltyCrossSize;


    FieldSize _size;

    /**
    * optimal position for goali to block ball
    * position is on the semicircle around goalcenter,
    * which is inside his penaltybox 
    */
    Coord centerpointGoaliDefenderCircle;
    float radiusGoaliDefenderCircle;

    // state: set -> manual placement!
    DirectedCoord getManualSetPose(int botId, bool hasKickoff) const;

    // state: ready -> target pos
    DirectedCoord getReadyPose(int botId, bool hasKickoff) const;

    // state: play -> re-enter posi(s)
    std::vector<DirectedCoord> getUnpenalizedPose() const;
    
    //if replaced in Set
    std::vector<DirectedCoord> getManualPlacementPose(bool kickoff, bool isGoali) const;

    // state: initial -> start posi(s)
    std::vector<DirectedCoord> getInitialPose() const;

    //Returns Außenkoordinaten von der Box in der der Bot laufen soll.
    Coord getPlayingBoxStart(RobotRole role);
    Coord getPlayinBoxEnd(RobotRole role) ;

    Coord getPenaltyMarkPosition(bool opponent = true) const;

    bool insidePenaltybox(const Coord &pos) const;

    /**
     *
     * Returns this length marked by X.
     *
     * |---------
     * |       |
     * |--|    |
     * |  |   (|)
     * |--|    |
     * |  X    |
     * |  X    |
     * ----------
     *
     */

    float penaltyBoxToFieldBorderLength() const;

    /**
     *
     * Returns this length marked by X.
     *
     * |---------
     * |       |
     * |--|    |
     * |  |   (|)
     * |--|XXXX|
     * |       |
     * |       |
     * ----------
     *
     */
    float penaltyBoxToMiddleLineLength() const;

    /**
     * returns all possible distances and angles in RCS to a
     * given directed coord.
     * currently we use this to calculate dists and angles
     * for every particle in the localization filter.
     * type can be ommited, if a special landmark is requested,
     * otherwise all landmark types are returned.
     * possible values for type:
     * 0: crosses type L (8x)
     * 1: crosses type T (6x)
     * 2: crosses type X (2x)
     * 3: poles (4x)
     */
    std::vector<LandmarkOrientation> getDistsAndAngles(const DirectedCoord &from,
            int type=-1);


    DirectedCoord kick_off_position(bool has_kickoff);

    bool ball_outside_centercircle(Coord ball);

    /**
    * returns optimal position for goali to block ball
    * position is on the semicircle around goalcenter,
    * which is inside his penaltybox 
    */
    DirectedCoord goaliDefenderPosition(Angle theta);

    Angle getCurrentThetaGoali(DirectedCoord bot_pos);

    /**
     * Returns the absolute width of the field include the area outside of
     * bounds from one corner to the other on one side of the field.
     */
    inline float getAbsoluteWidth() const { return _width; }
    /**
     * Returns the absolute length of the field including the area outside of
     * bounds from one goal to the other.
     */
    inline float getAbsoluteLength() const { return _length; }

private:
    /** struct for field measurements used by createField()
     * all dimensions are in meter, default values correspond
     * to current SPL rules
     */
    struct FieldMeasurements {
        float fieldLength{9.f};
        float fieldWidth{6.f};
        float centerCircleDiameter{1.5f};
        float penaltyCrossDistance{1.3f};
        float penaltyAreaLength{0.6f};
        float penaltyAreaWidth{2.2f};
        float goalWidth{1.5f};
        float goalPostDiameter{0.1f};
        float goalboxDepth{0.5f};
        float goalPostHeight{0.8f};
        float borderStripWidth{0.7f};
        float penaltyCrossSize{0.1f};
        float lineWidth{0.05f};
    };


    /**
     * create playingfield lines, crosses, etc.
     */
    void createField(const FieldMeasurements &fm);

    /** creates line equations all in wcs coords (m)
     * @param angle of line rad
     * @param starX startpoint x
     * @param startY || y
     * @param endX endpoint x
     * @param end Y || y
     * @param lineType name of line from enum Line
     */
    void createNewLineDefinition(
        const float alpha,
        const float startX,
        const float startY,
        const float endX,
        const float endY,
        const Line lineType);



    /** creates Corner/Crosses of wcs in m
     * @param x coord
     * @param y coord
     * @param alpha rad
     * @param degree type 2,3,4 (L,T,X)
     * @param crossType name enum Cross
     */
    void createNewCrossDefinition(const float x,
                                  const float y,
                                  const float alpha,
                                  const int degree,
                                  const Cross crossType);

    /** creates Poles in WCS (m)
     * @param x coord
     * @param y cooed
     * @param height of pole
     * @param width of pole
     * @param color enum from vision
     * @param poleType enum Pole
     */
    LandmarkPole createNewPoleDefinition(const float x,
                                         const float y,
                                         const float height,
                                         const float width,
                                         const int color,
                                         const Pole poleType);

    /** creates Goals in WCS (m)
     * @param left Pole
     * @param right Pole
     * @param poleDistance (m)
     * @param opponent 0=own;1=opponent
     * */
    void createNewGoalDefinition(const LandmarkPole &left,
                                 const LandmarkPole &right,
                                 const float poleDistance,
                                 const bool opponent);


    /**
     * creates a vector for all crosses where all distances to all crosses are included (also the own, which schould be 0)
     * the index of the vector elemts matches the enum Cross
     */
    void createDistVecOfCrosses();

};

// vim: set ts=4 sw=4 sts=4 expandtab:
