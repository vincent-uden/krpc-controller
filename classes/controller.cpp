#include "../headers/controller.h"

Controller::Controller(
        krpc::Client &conn, 
        krpc::services::KRPC &krpc, 
        krpc::services::SpaceCenter &sc): 
    conn(conn), krpc(krpc), sc(sc) {

    vessel = sc.active_vessel();

}

Controller::~Controller() {

}

void Controller::setFlightStage(int index) {
    flightStage = index;
    printFlightStage(flightStage);
}

void Controller::printFlightStage(int index) {
    std::cout << "Current Flight Stage:\n  "
        << flightStageNames[index] 
        << std::endl;
}

void Controller::startControl() {

}

Probe1Controller::Probe1Controller(
        krpc::Client &conn, 
        krpc::services::KRPC &krpc, 
        krpc::services::SpaceCenter &sc): 
    Controller(conn, krpc, sc) {
    flightStageNames.push_back("Take Off");
    flightStageNames.push_back("Ascent");
    flightStageNames.push_back("Waiting for Circularization");
    flightStageNames.push_back("Circularization Burn");
}

Probe1Controller::~Probe1Controller() {

}

void Probe1Controller::startControl() {
    vessel.auto_pilot().target_pitch_and_heading(90, 270);
    vessel.auto_pilot().engage();
    vessel.control().set_throttle(1);
    setFlightStage(0);
    vessel.control().activate_next_stage();

    auto orbit = vessel.orbit();
    auto flight = vessel.flight();
    auto resources = vessel.resources();

    auto solidFuelObj = resources.with_resource("SolidFuel");

    auto names = resources.names();
    std::cout << "Vessel contains the following fuels:" << std::endl;
    for (int i = 0; i < names.size(); i++) {
        std::cout << names[i] << std::endl;
    }
    std::cout << "####################################\n" << std::endl;


    setFlightStage(1);
    startLogging();

    while (true) {
        height = flight.mean_altitude();
        apoapsis = orbit.apoapsis_altitude();
        periapsis = orbit.periapsis_altitude();
        solidFuelObj = resources.with_resource("SolidFuel");
        if (solidFuelObj.size() > 0) {
            solidFuel = solidFuelObj[0].amount();
        }
        
        controlPitch();
        controlThrottle();
        controlStaging();
    }
}

void Probe1Controller::controlPitch() {
    if (logging)
        log();
    switch (flightStage) {
    case 1:
        vessel.auto_pilot().target_pitch_and_heading(
            ascentProfile(height), orbitHeading
        );
        break;
    default:
        vessel.auto_pilot().target_pitch_and_heading(
            ascentProfile(height), orbitHeading
        );
    }
}

void Probe1Controller::controlThrottle() {
    switch (flightStage) {
    case 1:
        if (apoapsis > kerbinOrbitTarget) {
            vessel.control().set_throttle(0);
            if (height > 70000) {
                stopLogging();
                setFlightStage(2);
                startLogging();
            }
        } else {
            vessel.control().set_throttle(1);
        }
        break;
    case 2:
        if (apoapsis - height < 1000 && !hasCircularized) {
            hasCircularized = true;
            stopLogging();
            setFlightStage(3);
            startLogging();
            vessel.control().set_throttle(1);
        }
        break;
    case 3:
        // Somethings wrong
        if (apoapsis - periapsis < 5000) {
            vessel.control().set_throttle(0);
        } else {
            double x = (kerbinOrbitTarget - std::max(periapsis, (double) 0)) / kerbinOrbitTarget;
            vessel.control().set_throttle(x);
        }
        break;
    }
}

void Probe1Controller::controlStaging() {
    switch (flightStage) {
    case 1:
        if (solidFuel == 0 && !hasDroppedBoosters) {
            hasDroppedBoosters = true;
            vessel.control().activate_next_stage();
        }
    }
}

void Probe1Controller::log() {
    if (timer == 0) {
        std::cout 
            << "\r                                                                              \r" 
            << std::flush;
        std::cout 
            << "Height: " << height
            << " Angle: " << 180 - 90 * exp((-height) / 20000.0) 
            << " Apoapsis: " << apoapsis
            << " Periapsis: " << periapsis
            << " Solid Fuel: " << solidFuel
            << std::flush;
    }
    timer++;
    timer %= 5;
}

void Probe1Controller::startLogging() {
    logging = true;
}

void Probe1Controller::stopLogging() {
    logging = false;
    std::cout 
        << "\r                                                                                       \r" 
        << std::flush;
}

double Probe1Controller::ascentProfile(double height) {
    return 180 - 90 * exp((-height) / 20000.0);
}
