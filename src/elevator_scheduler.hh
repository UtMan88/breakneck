#ifndef ELEVATOR_SCHEDULER_HH
#define ELEVATOR_SCHEDULER_HH

#include <vector>
#include <elevator.hh>

class ElevatorScheduler {
    // struct ElevatorInfo {
    //     Elevator::FloorDiffType dist;
    //     Elevator::Queue::size_type queueSz;
    //     Elevator* elev;
    // };
    typedef std::pair<Elevator::FloorDiffType, Elevator*> DiffElevPair;

    std::vector<std::unique_ptr<Elevator>> mElevators;

  public:
    ElevatorScheduler();

    ElevatorScheduler(const ElevatorScheduler&) = delete;

    ElevatorScheduler& operator=(const ElevatorScheduler&) = delete;

    ElevatorScheduler(ElevatorScheduler&&) = delete;

    ElevatorScheduler& operator=(ElevatorScheduler&&) = delete;

    ~ElevatorScheduler() = default;

    bool requestElevator(int floorNum, const Elevator::Direction& dir);
};

#endif // ELEVATOR_SCHEDULER_HH
