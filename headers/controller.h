#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <krpc.hpp>
#include <krpc/services/krpc.hpp>
#include <krpc/services/space_center.hpp>

class Controller {
public:
    Controller(krpc::Client &conn, krpc::services::KRPC &krpc, krpc::services::SpaceCenter &sc);
    ~Controller();
    
    void setFlightStage(int index);

    virtual void startControl();

protected:
    krpc::Client &conn;
    krpc::services::KRPC &krpc;
    krpc::services::SpaceCenter &sc;

    krpc::services::SpaceCenter::Vessel vessel;

    int flightStage = 0;
    std::vector<std::string> flightStageNames;

    void printFlightStage(int index);
};

// Flight Stages
// 0 - Take off
// 1 - Ascent
// 2 - Wait for circularization
// 3 - Circularization burn
// 4 - Orbiting
class Probe1Controller : public Controller {
public:
    Probe1Controller(krpc::Client &conn, krpc::services::KRPC &krpc, krpc::services::SpaceCenter &sc);
    ~Probe1Controller();

    void startControl();
    void controlPitch();
    void controlThrottle();
    void controlStaging();
    void startLogging();
    void stopLogging();

    double ascentProfile(double height);

private:
    double height;
    double apoapsis;
    double periapsis;
    double solidFuel;
    double kerbinOrbitTarget = 100000;
    double orbitHeading = 270;

    int timer = 0;

    bool logging = false;
    bool hasDroppedBoosters = false;
    bool hasCircularized = false;
     
    void log();
};
