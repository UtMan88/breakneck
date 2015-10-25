#include <cstdlib>
#include <elevator_scheduler.hh>

typedef std::pair<Elevator::FloorDiffType, Elevator*> DiffElevPair;

ElevatorScheduler::ElevatorScheduler() {
    mElevators.emplace_back(new Elevator(1, 1, 1, 32));
    mElevators.emplace_back(new Elevator(2, 1, 1, 32));
    mElevators.emplace_back(new Elevator(3, 8, 1, 32));
    mElevators.emplace_back(new Elevator(4, 16, 1, 32));
    assert(mElevators.size() > 0 && "ElevatorScheduler must have an elevator");
}

bool ElevatorScheduler::requestElevator(int floorNum, const Elevator::Direction& dir) {
    auto pairs = sortByNearest(floorNum);
    auto itr = std::find_if(pairs.begin(), pairs.end(), [&](DiffElevPair& pair) {
        auto* elev = pair.second;
        if (dir != elev->getDirection()) {
            return false;
        }
        return static_cast<bool>(elev->addFloorIfInPath(floorNum));
    });
    if (itr != pairs.end()) {
        return true;
    }
    itr = std::find_if(pairs.begin(), pairs.end(), [floorNum](DiffElevPair& pair) {
        return static_cast<bool>(pair.second->addFloor(floorNum, false));
    });
    return itr != pairs.end();
}

std::vector<DiffElevPair>
ElevatorScheduler::sortByNearest(int floorNum) const {
    std::vector<DiffElevPair> elevators;
    std::for_each(mElevators.cbegin(), mElevators.cend(),
                  [&](const std::unique_ptr<Elevator>& elev) {
        auto dist = std::abs(elev->getLastQueuedFloor() - floorNum);
        elevators.push_back(std::make_pair(dist, elev.get()));
    });
    std::sort(elevators.begin(), elevators.end(),
              [](const DiffElevPair& p1, const DiffElevPair& p2) {
        return p1.first < p2.first;
    });
    return elevators;
}
