#include <cstdlib>
#include <elevator_scheduler.hh>

namespace {

typedef std::pair<Elevator::FloorDiffType, Elevator*> DiffElevPair;

std::vector<DiffElevPair> sortByNearest(
      const std::vector<std::unique_ptr<Elevator>>& elevators,
      int floorNum) {
    std::vector<DiffElevPair> pairs;
    std::for_each(elevators.cbegin(), elevators.cend(),
                  [&](const std::unique_ptr<Elevator>& elev) {
        auto dist = std::abs(elev->getLastQueuedFloor() - floorNum);
        pairs.push_back(std::make_pair(dist, elev.get()));
    });
    std::sort(pairs.begin(), pairs.end(),
              [](const DiffElevPair& p1, const DiffElevPair& p2) {
        return p1.first < p2.first;
    });
    return pairs;
}

} // end namespace

ElevatorScheduler::ElevatorScheduler() {
    mElevators.emplace_back(new Elevator(1, 1, 1, 32));
    mElevators.emplace_back(new Elevator(2, 1, 1, 32));
    mElevators.emplace_back(new Elevator(3, 8, 1, 32));
    mElevators.emplace_back(new Elevator(4, 16, 1, 32));
    assert(mElevators.size() > 0 && "ElevatorScheduler must have an elevator");
}

bool ElevatorScheduler::requestElevator(int floorNum, const Elevator::Direction& dir) {
    auto pairs = ::sortByNearest(mElevators, floorNum);
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