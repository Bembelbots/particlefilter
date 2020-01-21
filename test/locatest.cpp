#include <coords.h>
#include <particlefilter.h>
#include <definitions.h>
#include <logdataprocessor.hpp>

using namespace std;

#define TEXT_NORMAL "\033[0m"
#define TEXT_HEADLINE "\033[0;31m"

int BOTTOM_CAM = 1;
int TOP_CAM = 0;

int L_CROSSING = 2;
int T_CROSSING = 3;
int X_CROSSING = 4;

class ParticleFilterTest {
public:
    /*
    set
    - robot_id
    - has_kockoff
    - role
    - num_particles
    - odo_variance
    - odo_start
    - start_position
    */
    FieldSize fieldSize = FieldSize::JRL;
    ParticleFilter::Settings conf;

    ParticleFilterTest():
    conf(new PlayingField(fieldSize)){}

    int runParticleFilter(LogDataset dataset, string outFile) {
        conf.numParticles = 50;
        conf.robot_id = 2;
        ParticleFilter loca(conf);
        int cognition_step = 0; 
        vector<Robot> positionOutputsAndTargets; //for erroranalyzis
        cout << "run particle filter on field with length :" <<conf.pf->_lengthInsideBounds<<endl;

        loca.emit_event(ParticleFilter::EV_INTIAL);
        DirectedCoord odometry(0.0f, 0.0f, 0.0f);
        for (LogData &data : dataset.data) {
            vector<VisionResult> vrs;
            cognition_step++;
            dataset.output.push_back(data);

            //Events:
            for (auto event : data.event ) {
                cout<< "Emit event "<< (int)(event)<<endl;;
                loca.emit_event(event);
            }

            //Odometrie:
            if (!data.odo.empty()) {
                odometry = DirectedCoord(data.odo.at(0));
            }
            else{
                cout << "No Odometry found at timestamp: "<<cognition_step <<endl;
            }

            //poseEstimates:
            pair<vector<DirectedCoord>,int> poseEstimates={{},1};
            for (auto r: data.poseEstimates){
                dataset.output.at(dataset.output.size() -1 ).poseEstimates.push_back(r);
                poseEstimates.first.push_back(r.pos);
            }

            //update Loca
            loca.update(data.visionResults, odometry, poseEstimates);
            
            //getPos:
            Robot outpos(loca.get_position(),data.wcs.GTpos);
            positionOutputsAndTargets.push_back(outpos);
            dataset.output.at(dataset.output.size() -1).wcs = outpos;

            //get partikels
            vector<DirectedCoord> particles = loca.getHypothesesVector();
            for (auto h: particles) {
                Robot r;
                r.pos = h;
                r.confidence = 1;
                dataset.output.at(dataset.output.size() -1).hypos.push_back(r);
            }
        }
        cout <<  TEXT_HEADLINE << "EVALUTAION OF LOCA FILTER, mean dist: "<<
                mean(evaluatePositionsWithGroundtruth(positionOutputsAndTargets))<<
                TEXT_NORMAL<< endl;

        cout <<"SAVE DATA IN LOGFILE"<<outFile<< endl;
        dataset.logtoFile(outFile);
        
        cout << TEXT_HEADLINE << "end of test"<< endl;
        cout << "---------------------------------------------------------"
                 << TEXT_NORMAL<< endl;

        return 0;
    }

    // to show initial/penalized/manPlacement handler
    
    int initFilter(std::string filename) {
        ofstream out(filename);
        DirectedCoord odometry(0.0f, 0.0f, 0.0f);

        conf.has_kickoff = true;
        ParticleFilter loca(conf);
        for (int type =0; type <=6;type++){//type =0-5 : normal for id type //6:: penalizes //7:: manualPlacement
            if ((type >= 0)and (type <= 4)){
                loca.conf.robot_id = type;
                loca.emit_event(ParticleFilter::EV_STATE_INITIAL);
            }
            if(type == 5) {
                loca.unpenalizedHandler();  
            }
            if(type == 6) {
                loca.manualPlacementHandler(); 
            }
            
            out<< "(" << type<< ")" << " enter new cognition step: " << type << endl;

            Robot outpos(loca.get_position());

            // write hypos
            vector<DirectedCoord>  hypos = loca.getHypothesesVector();
            for (auto h: hypos) {
                Robot r;
                r.pos = h;
                r.confidence = 1;
                out << "(" << type<< ") Hypo: "  << r << endl; 
            }
            // write  estimated position
            out << "(" << type << ") WCS: " << outpos << endl; 
        }
        out.close();
        return 0;
    }

private:
    float mean( vector<float> v) {
        float sum =  accumulate(v.begin(), v.end(), 0.0);
        float mean = sum / v.size();
        return mean;
    }

     vector<float>  evaluatePositionsWithGroundtruth(vector<Robot> positions) {
        vector<float> dist;
        vector<float> angledist;
        float stdev;
        for (size_t i=0; i < positions.size(); i++) {
            Coord pos = Coord(positions.at(i).pos.coord.x, positions.at(i).pos.coord.y);
            Coord GTpos = Coord(positions.at(i).GTpos.coord.x,
                                positions.at(i).GTpos.coord.y);
            //distance
            dist.push_back(pos.dist(GTpos));
            //angle
            float alpha = positions.at(i).pos.angle.rad;
            float GTalpha = positions.at(i).GTpos.angle.rad;
            if ((alpha>0 and GTalpha<0) or ((alpha < 0 and GTalpha > 0))) {
                angledist.push_back(min(alpha+GTalpha, 2*M_PI_F-(alpha+GTalpha)));
            } else {
                angledist.push_back(fabsf(fabsf(alpha)- fabsf(GTalpha)));
            }
        }
        return dist;
    }
};


int main(int argc, const char *argv[]) {

    cout<< "hi, this is the loca testbox" << TEXT_NORMAL<<endl;
    ParticleFilterTest test;

    string data_fn;
    if (argc > 1 && argv[1] != NULL) {
        data_fn =  string(argv[1]);
        //parse logfile
        LogDataset log(data_fn);
        if (log.data.size() >0){
            return test.runParticleFilter(log, "ParticleFilterOutput.log");
        }
        else{
            cout <<  TEXT_HEADLINE << "Couldn't read cognitionsteps from logfile!! :/"<<
                TEXT_NORMAL<< endl;
            return 1;
        }
    } 
    else {
        cout<< TEXT_HEADLINE <<  "no logfile provided, test initializations"<<TEXT_NORMAL<<endl;
        return test.initFilter("ParticleFilterOutput.log");
    }
}

/* Code to save matched Landmarks instead of visionResults

dataset.output.at(dataset.output.size() -1).visionResults.clear();//LogData ld;
std::vector<Feature> matchedLandmarks= loca.matchedLandmarks;
for (auto landmark: matchedLandmarks){
    if (landmark.type = JSVISION_LINE){
        LandmarkLine pfLine = conf.pf->_lines[landmark.id];
        VisionResult vrsLine;
        vrsLine.type= JSVISION_LINE;
        auto rcs1= DirectedCoord(pfLine.start_x,pfLine.start_y,0).toRCS(outpos.GTpos);
        vrsLine.rcs_x1= rcs1.coord.x;
        vrsLine.rcs_y1= rcs1.coord.y;
        auto rcs2= DirectedCoord(pfLine.end_x,pfLine.end_y,0).toRCS(outpos.GTpos);
        vrsLine.rcs_x2= rcs2.coord.x;
        vrsLine.rcs_y2= rcs2.coord.y;
        cout <<"line:"<< landmark.id
             <<",pos:"<<Coord(pfLine.start_x,pfLine.start_y)
             <<"robot at:"<< outpos.pos
             <<",rcspos:"<< rcs1.coord<<endl;

        dataset.output.at(dataset.output.size() -1).visionResults.push_back(vrsLine);
    }
}*/ 
