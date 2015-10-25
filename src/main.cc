#include <atomic>
#include <cstdlib>
#include <condition_variable>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <boost/optional.hpp>

//#define NDEBUG

#define DISALLOW_ALL_COPY_AND_MOVE(TypeName) \
    TypeName(const TypeName&);               \
    TypeName& operator=(const TypeName&);    \
    TypeName(TypeName&&);                    \
    TypeName& operator=(TypeName&&)

namespace {
std::mutex ioMutex;
}

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
    typedef std::vector<int> Queue;
    typedef Queue::difference_type FloorDiffType;
  private:
    Queue mNextFloorQueue; // INVARIANT must always be sorted

  public:
    enum class Direction { Up, Down, Stand, Maintenance };
  private:
    Direction mDirection;

  private:
    DISALLOW_ALL_COPY_AND_MOVE(Elevator);

    void goToFloor(int num) {
        auto floor = getLastQueuedFloor();
        std::unique_lock<std::mutex> guard(mMutex);
        std::unique_lock<std::mutex> ioGuard(::ioMutex);
        std::cout << mId << ": " << floor << " -> " << num << '\n';
        ioGuard.unlock();
        mNextFloorQueue.emplace_back(num);
        std::sort(mNextFloorQueue.begin(), mNextFloorQueue.end());
        guard.unlock();
        mCondVar.notify_one();
    }

    boost::optional<FloorDiffType>
    distToQueuedFloor(int num) const {
        std::unique_lock<std::mutex> guard(mMutex);
        auto itr = std::find(mNextFloorQueue.cbegin(), mNextFloorQueue.cend(), num);
        if (itr == mNextFloorQueue.cend()) {
           return boost::none;
        }
        auto dist = std::distance(itr, mNextFloorQueue.cend());
        guard.unlock();
        return boost::make_optional(std::abs(dist));
    }

    void moveTo(int destFloor, const Direction& dir) {
        mDirection = dir;
        for (; destFloor < mFloorNum;) {
            switch (mDirection) {
              case Direction::Up:
                // std::this_thread::sleep_for(std::chrono::seconds(1));
                ++mFloorNum;
                break;
              case Direction::Down:
                // std::this_thread::sleep_for(std::chrono::seconds(1));
                --mFloorNum;
                break;
              case Direction::Maintenance:
              case Direction::Stand:
                assert(destFloor == mFloorNum && "Cannot move in Stand/Maintenance");
                break;
              default:
                assert(false);
            }
        }
        assert(validFloor(mFloorNum) && "Cannot move to floor");
    }

    void openDoor() {
        std::lock_guard<std::mutex> guard(::ioMutex);
        std::cout << mId << ": " << mFloorNum << '\n';
        //std::this_thread::sleep_for(std::chrono::seconds(1));
        //closeDoor();
    }

    bool validFloor(int num) const {
        return num >= mMinFloor && num <= mMaxFloor;
    }

    void start() {
        for (;;) {
            std::unique_lock<std::mutex> guard(mMutex);
            mCondVar.wait(guard);
            if (mJoinThread) {
                break;
            }
            // TODO: should probably create a queue from two vectors
            assert(mNextFloorQueue.size() > 0 && "Queue cannot be empty");
            auto destFloor = mNextFloorQueue.front();
            mNextFloorQueue.erase(mNextFloorQueue.cbegin());
            guard.unlock();
            if (destFloor < mFloorNum) {
                moveTo(destFloor, Direction::Down);
            } else if (destFloor > mFloorNum) {
                moveTo(destFloor, Direction::Up);
            }
            openDoor();
        }
        mCondVar.notify_one();
    }

  public:
    Elevator(int id, int startFloor, int minFloor, int maxFloor)
        : mJoinThread(false), mThread(&Elevator::start, this)
        , mId(id), mFloorNum(startFloor), mMinFloor(minFloor)
        , mMaxFloor(maxFloor) , mDirection(Direction::Stand) {
        assert(minFloor < maxFloor && "minFloor must be less than maxFloor");
        assert(validFloor(startFloor) && "startFloor not within range");

        mNextFloorQueue.reserve(static_cast<Queue::size_type>(maxFloor-minFloor));
    }

    ~Elevator() {
        std::unique_lock<std::mutex> guard(mMutex);
        mJoinThread = true;
        mCondVar.notify_one();
        mCondVar.wait(guard);
        mThread.join();
    }

    Direction getDirection() const { return mDirection; }

    int getLastQueuedFloor() const {
        std::unique_lock<std::mutex> guard(mMutex);
        if (mNextFloorQueue.size() > 0) {
            return mNextFloorQueue.back();
        } else {
            return mFloorNum;
        }
    }

    boost::optional<FloorDiffType>
    addFloor(int floorNum, bool now) {
        if (!validFloor(floorNum)) {
            return boost::none;
        }
        if (now) {
            std::unique_lock<std::mutex> guard(mMutex);
            if (mNextFloorQueue.front() != floorNum) {
                std::remove(mNextFloorQueue.begin(), mNextFloorQueue.end(), floorNum);
                mNextFloorQueue.emplace(mNextFloorQueue.begin(), floorNum);
            }
            guard.unlock();
            mCondVar.notify_one();
            return boost::make_optional(0L);
        } else {
            if (auto dist = distToQueuedFloor(floorNum)) {
                return dist;
            }
            goToFloor(floorNum);
            return distToQueuedFloor(floorNum);
        }
    }

    boost::optional<FloorDiffType>
    addFloorIfInPath(int destFloor) {
        if (!validFloor(destFloor)) {
            return boost::none;
        }
        if (auto dist = distToQueuedFloor(destFloor)) {
            return dist;
        }
        if (mDirection == Direction::Stand
            || destFloor == mMinFloor || destFloor == mMaxFloor
            || (destFloor < mFloorNum && mDirection == Direction::Down)
            || (destFloor > mFloorNum && mDirection == Direction::Up)) {
            goToFloor(destFloor);
            return distToQueuedFloor(destFloor);
        }
        return boost::none;
    }
};

class ElevatorScheduler {
    struct ElevatorInfo {
        Elevator::FloorDiffType dist;
        Elevator::Queue::size_type queueSz;
        Elevator* elev;
    };
    typedef std::pair<Elevator::FloorDiffType, Elevator*> DiffElevPair;

    std::vector<std::unique_ptr<Elevator>> mElevators;

  private:
    DISALLOW_ALL_COPY_AND_MOVE(ElevatorScheduler);

    std::vector<DiffElevPair>
    sortByNearest(int floorNum) const {
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

  public:
    ElevatorScheduler() {
        mElevators.emplace_back(new Elevator(1, 1, 1, 32));
        mElevators.emplace_back(new Elevator(2, 1, 1, 32));
        mElevators.emplace_back(new Elevator(3, 8, 1, 32));
        mElevators.emplace_back(new Elevator(4, 16, 1, 32));
        assert(mElevators.size() > 0 && "ElevatorScheduler must have an elevator");
    }

    bool requestElevator(int floorNum, const Elevator::Direction& dir) {
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
};

class ElevatorBankPanel {
    const int mFloorNum;
    ElevatorScheduler* const mElevatorScheduler;

  public:
    enum class Direction { Up, Down };

    ElevatorBankPanel(int floorNum, ElevatorScheduler* const scheduler)
        : mFloorNum(floorNum), mElevatorScheduler(scheduler) {
        assert(scheduler && "ElevatorScheduler cannot be null");
    }

    ElevatorBankPanel(const ElevatorBankPanel&) = delete;
    ElevatorBankPanel& operator=(const ElevatorBankPanel&) = delete;

    ElevatorBankPanel(ElevatorBankPanel&& other)
        : mFloorNum(other.mFloorNum)
        , mElevatorScheduler(other.mElevatorScheduler) {
    }

    ElevatorBankPanel& operator=(ElevatorBankPanel&&) = delete;

    bool pushButton(const Direction& dir) {
        typedef Elevator::Direction Dir;
        if (dir == Direction::Up) {
            return mElevatorScheduler->requestElevator(mFloorNum, Dir::Up);
        }
        if (dir == Direction::Down) {
            return mElevatorScheduler->requestElevator(mFloorNum, Dir::Down);
        }
        return false;
    }
};

int main(int argc, char** argv) {
    // Order is important, vector must be destroyed before ElevatorScheduler!
    std::unique_ptr<ElevatorScheduler> scheduler(new ElevatorScheduler());
    typedef std::vector<ElevatorBankPanel> List;
    List panels;

    panels.reserve(32);
    {
        auto* pScheduler = scheduler.get();
        for (int num = 0; num < 32; ++num) {
            panels.emplace_back(num+1, pScheduler);
        }
    }

    std::random_device randSeed;
    std::default_random_engine gen(randSeed());
    std::uniform_int_distribution<List::size_type> dis(0, 31);

    for (;;) {
        auto floor = dis(gen);
        if (floor % 2 == 0) {
            panels.at(floor).pushButton(ElevatorBankPanel::Direction::Down);
        } else {
            panels.at(floor).pushButton(ElevatorBankPanel::Direction::Up);
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen) * 100));
    }

    return 0;
}
