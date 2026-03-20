#include "general/common.h"
#include "general/interfaces/event_loop/timer_wheel.h"
#include "general/interfaces/storage/allocator.h"
#include "general/misc/errors.h"
#include <cassert>
#include <cmath>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <expected>

enum WheelNodeType {
  TIMEOUT,
  ACTION,
};
template <size_t layer_count> struct WheelNode {
  WheelNode *next = nullptr;
  WheelNode *pre = nullptr;
  std::coroutine_handle<> handle;
  size_t layer_indices[layer_count];
};

template <size_t layer_size, size_t layer_count>
class LayerWheel : public WheelInterface {
private:
  uint64_t total_ticks;
  size_t cursors[layer_count] = {};
  AllocatorInterface *wheelnode_allocator;
  WheelNode<layer_count> *layers[layer_size][layer_count] = {};
  size_t calculate_layers(uint64_t remaining_tick,
                          WheelNode<layer_count> *wheel_node);

  std::expected<void, int> cascade(size_t node_index, size_t layer_index);

  std::expected<void, int> activate(size_t node_index);

public:
  LayerWheel(AllocatorInterface *wheelnode_allocator);
  std::expected<void, int> add_node(std::coroutine_handle<> handle,
                                    uint64_t remaining_tick) override;
  std::expected<void, int> tick() override;
};
