#include <iostream>
#include <string>
#include <fstream>
#include <definitions.h>
#include <coords.h>
#include <particlefilter.h>
#include <random>

struct LogData {
    int stamp;
    std::vector<Robot> poseEstimates;
    Robot wcs;
    std::vector<Robot> hypos;
    std::vector<DirectedCoord> odo;
    std::vector<VisionResult> visionResults;
    std::vector<std::string> s;
    std::vector<ParticleFilter::tLocalizationEvent> event;
};

class LogDataset {
public:
    std::vector<LogData> data;
    std::vector<LogData> output; //if multiple visualizations per output are needed

    LogDataset(const std::string &fn){
        std::ifstream input(fn);
        LogData ld; //one cognition step in dataset
        for (std::string line; std::getline(input, line);) {
            if (line.size() < 3) {
                continue;
            }
            if (line.find("(") != 0) {
                continue;
            }

            int stamp = atoi(line.substr(1, line.find(")")).c_str());
            ld.stamp = stamp;

            std::string s = line.substr(line.find(")") + 2);
            if (s.find("enter new cognition step:") == 0) {                    
                int step = atoi(s.substr(s.find(": ") + 2).c_str());
                // add as new log data element
                data.push_back(ld);
                ld = LogData();
                continue;
            }

            if (s.find("WCS: ") != std::string::npos) {
                ld.wcs.setFromString(s.substr(5));
                continue;
            }

            if (s.find("Odometry: ") != std::string::npos) {
                size_t pos1 = s.find(";");
                size_t pos2 = s.find(";", pos1 + 1);
                float x = atof(s.substr(10, pos1 - 10).c_str());
                float y = atof(s.substr(pos1 + 1, pos2 - pos1 - 1).c_str());
                float a = atof(s.substr(pos2 + 1).c_str());
                ld.odo.push_back(DirectedCoord(x, y, a));
                continue;
            }

            if (s.find("Measurement: ") != std::string::npos) {
                Robot r;
                r.setFromString(s.substr(13));
                ld.poseEstimates.push_back(r);
                continue;
            }
            if (s.find("VisionResult: ") != std::string::npos) {
                VisionResult vr(s.substr(14));
                ld.visionResults.push_back(vr);
                continue;
            }
            if (s.find("Emit event: ") != std::string::npos) {
                size_t pos = s.find(":");
                ld.event.push_back(static_cast<ParticleFilter::tLocalizationEvent>(atoi(s.substr(pos+1).c_str())));
                continue;
            }
        }

        std::cout << "read " << data.size() << " cognition steps from logfile " << std::endl;
    }

    void logtoFile(const std::string  filename){
        std::ofstream out(filename);
        int cognition_step = 0;
        for (auto d: output) {
            if (cognition_step %500 == 0){
                std::cout<<cognition_step <<std::endl;
            }
            //start output with new declaration of cognition step
            out << "(" << d.stamp << ")" << " enter new cognition step: "<< cognition_step << std::endl;
            
            //outpus Robot position 
            out << "(" << d.stamp << ") WCS: "<< d.wcs << std::endl;
            
            //output vision results
            for (auto vr : d.visionResults){
                out<< "(" << d.stamp << ") " << "VisionResult: " << vr << std::endl;
            } 
            //outpus measurement( pose estimations from landmarks)  
            for (auto meas : d.poseEstimates){
                out << "(" << d.stamp << ") Measurement: "<< meas << std::endl;
            }
            //output particle filter hypos
            for (auto hypo : d.hypos){
                out << "(" << d.stamp << ") Hypo: "<< hypo << std::endl;
            }
            //output odometry
            for (auto odo:d.odo) {
                out << "(" << d.stamp << ") Odometry: "<< odo.coord.x << ";" 
                    << odo.coord.y <<  ";" << odo.angle.rad << std::endl;
       
            }
            //output localizationevent
            for (auto event : d.event){
                out << "(" << d.stamp << ") Emit event: "<< (int) event << std::endl;
            }
            cognition_step ++;            
        }
        out.close();
    }

};