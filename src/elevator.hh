#ifndef ELEVATOR_HH
#define ELEVATOR_HH

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>
#include <boost/optional.hpp>

class Elevator final {
    std::condition_variable mCondVar;
    mutable std::mutex mMutex;
    bool mJoinThread;
    std::thread mThread;

    const int mId;
    std::atomic_int mFloorNum;
    const int mMinFloor;
    const int mMaxFloor;

  public:
    enum class Direction { Up, Down, Stand, Maintenance };

    typedef std::vector<int> Queue;
    typedef Queue::difference_type FloorDiffType;
  private:
    // TODO: remove, it can be derived from getDistanceTo
    Direction mDirection;

    Queue mNextFloorQueue; // INVARIANT must always be sorted

  private:
    boost::optional<FloorDiffType> distToQueuedFloor(int num) const;

    void goToFloor(int num);

    void moveTo(int destFloor, const Direction& dir);

    void openDoor();

    void start();

    bool validFloor(int num) const;

  public:
    Elevator(int id, int startFloor, int minFloor, int maxFloor);

    Elevator(const Elevator&) = delete;

    Elevator& operator=(const Elevator&) = delete;

    Elevator(Elevator&&) = delete;

    Elevator& operator=(Elevator&&) = delete;

    ~Elevator();

    boost::optional<FloorDiffType> addFloor(int floorNum, bool now);

    boost::optional<FloorDiffType> addFloorIfInPath(int destFloor);

    boost::optional<FloorDiffType>
    distToFloor(const int num, const Direction&) const;

    Direction getDirection() const;
};

inline Elevator::Direction Elevator::getDirection() const {
    return mDirection;
}

#endif // ELEVATOR_HH
