#include <iostream>
#include <chrono>
#include <thread>
#include <krpc.hpp>
#include <krpc/services/krpc.hpp>
#include <krpc/services/space_center.hpp>

#include "../headers/controller.h"

int main() {
    krpc::Client conn("Linux Conn", "109.228.160.84");
    krpc::services::KRPC krpc(&conn);
    std::cout << "Connected to kRPC server version " << krpc.get_status().version() << std::endl;
    krpc::services::SpaceCenter sc(&conn);

    Probe1Controller controller(conn, krpc, sc);
    //std::this_thread::sleep_for(std::chrono::seconds(2));
    controller.startControl();
}
