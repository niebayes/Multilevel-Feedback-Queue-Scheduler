// a1234567, Stark, Tony
// a2345678, Rogers, Steve
// a3456789, Banner, Bruce
// avengers

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <queue>
#include <sstream>
#include <vector>

class Customer {
 public:
  const int id;
  int priority;
  const int arrival_time;
  const int burst_time;
  int ticks;  // number of ticks this custormer has been scheduled.

  Customer(const int id, const int priority, const int arrival_time,
           const int slots_remaining)
      : id{id},
        priority{priority},
        arrival_time{arrival_time},
        burst_time{slots_remaining},
        ticks{0} {}

  // return true if this customer has run out of its burst time.
  bool Done() const { return ticks >= burst_time; }

  // comparator for sorting customers by arrival time and further by burst time.
  struct cmp {
    bool operator()(const Customer *a, const Customer *b) const {
      assert(a != nullptr && b != nullptr);
      if (a->arrival_time == b->arrival_time) {
        return a->burst_time >= b->burst_time;
      }
      // note, std::priority_queue's comparing order is creepy.
      return a->arrival_time > b->arrival_time;
    }
  };
};

// two-level feedback queue scheduler.
std::queue<Customer *> que0;  // waiting queue for high-priority customers.
std::queue<Customer *> que1;  // waiting queue for low-priority customers.

void feed(Customer *c, const bool first) {
  if (first) {
    que0.push(c);
  } else {
    que1.push(c);
  }
}

void elevate() {
  while (!que1.empty()) {
    Customer *c = que1.front();
    que1.pop();
    que0.push(c);
  }
}

// get the next scheduled customer from waiting queues.
Customer *next() {
  Customer *c{nullptr};
  if (!que0.empty()) {
    c = que0.front();
    que0.pop();
  } else if (!que1.empty()) {
    c = que1.front();
    que1.pop();
  }
  return c;
}

// modify these factors to get the optimized configuration.

// higher this, total_wait_0 < total_wait_1.
const double elapsed_factor = 1.55;
// lower this, larger quantum, n_switches less.
const double quantum_factor = 3;
// true to turn on elevation.
const bool do_elevate = false;
// unclear yet, but useful.
const double elevate_factor = 1;

int quantum;
// #ticks since the start of this quantum.
// this timer only increments if the running_customer is not null.
int elapsed;

// high-priority customers use the machine more often.
bool quantum_timeout(const Customer *c) {
  if (c->priority == 0) {
    return elapsed >= elapsed_factor * quantum;
  } else {
    return elapsed >= quantum;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Provide input and output file names." << std::endl;
    return -1;
  }

  std::ifstream input_file(argv[1], std::ios::in);
  if (!input_file.is_open()) {
    std::cerr << "Failed to open the input file." << std::endl;
    return -1;
  }

  std::ofstream output_file(argv[2], std::ios::out);
  if (!input_file.is_open()) {
    std::cerr << "Failed to open the output file." << std::endl;
    return -1;
  }

  // read all customer info from the input file.
  std::vector<Customer *> customers;
  std::vector<int> burst_times;  // used for computing quantum.
  std::string line;
  while (std::getline(input_file, line)) {
    // skip the leading 'c' character.
    std::istringstream iss{line.substr(1)};
    assert(iss.good());

    int id, priority, arrival_time, burst_time;
    if (iss >> id >> priority >> arrival_time >> burst_time) {
      // drop customers whose burst_times are 0.
      if (burst_time > 0) {
        Customer *c = new Customer(id, priority, arrival_time, burst_time);
        customers.push_back(std::move(c));
        burst_times.push_back(burst_time);
      }

    } else {
      assert(false && "invalid input line");
    }
  }

  // feed customers to arrival queue.
  std::priority_queue<Customer *, std::vector<Customer *>, Customer::cmp>
      arrival_que;
  for (Customer *c : customers) {
    arrival_que.push(c);
  }

  // compute the round robin quantum.
  const int median_index = int(burst_times.size()) >> 1;
  auto n = std::next(burst_times.begin(), median_index);
  std::nth_element(burst_times.begin(), n, burst_times.end());
  // quantum = burst_times.at(median_index);
  quantum = std::accumulate(burst_times.cbegin(), burst_times.cend(), 0) /
            burst_times.size();
  quantum /= quantum_factor;
  // const int quantum = 14;

  // #ticks to elevate all customers in the low-priority queue to the
  // high-priority queue.
  const int elevate_timeout = elevate_factor * quantum;

  // the id of the customer taking the machine currently.
  Customer *running_customer{nullptr};

  // logical timer.
  int tick{0};
  // #ticks since the last elevation.
  int elevate_elapsed{0};

  for (;;) {
    // feed arrived customers to waiting queue.
    while (!arrival_que.empty()) {
      Customer *c = arrival_que.top();
      if (c->arrival_time <= tick) {
        feed(c, true);
        arrival_que.pop();
      } else {
        break;
      }
    }

    if (running_customer != nullptr) {
      // if the running customer has completed, release the machine.
      if (running_customer->Done()) {
        running_customer = nullptr;
      } else if (quantum_timeout(running_customer)) {
        // if the running customer has used one quantum, feed it back to the
        // waiting queue and release the machine.
        feed(running_customer, false);
        running_customer = nullptr;
        // reset quantum timer.
        elapsed = 0;
      }
    }

    if (do_elevate && elevate_elapsed >= elevate_timeout) {
      elevate_elapsed = 0;
      elevate();
    }

    // try to schedule the next customer when the machine is idle.
    if (running_customer == nullptr) {
      running_customer = next();
    }

    // write schedule info.
    const int running_id =
        running_customer != nullptr ? running_customer->id : -1;
    output_file << tick << ' ' << running_id << '\n';

    // check if all customers are served
    if (running_customer == nullptr && arrival_que.empty() && que0.empty() &&
        que1.empty()) {
      break;
    }

    // tick.
    ++tick;
    ++elevate_elapsed;
    if (running_customer != nullptr) {
      running_customer->ticks++;
      ++elapsed;
    }
  }

  // free all customers.
  for (Customer *c : customers) {
    free(c);
    c = nullptr;
  }

  // close files.
  input_file.close();
  output_file.close();

  return 0;
}
