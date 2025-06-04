#pragma once
/*******************************************************************************
 * \file
 * File: src/ui/nodes.hpp
 * Created: 05/30/25
 * Author: Umut Sevdi
 * Description:
 *
 * Project: umutsevdi/logic-circuit-simulator-2.git
 * License: GNU GENERAL PUBLIC LICENSE
 ******************************************************************************/

#include <imnodes.h>

#include "common.h"

namespace lcs::ui {

template <typename T> class NodeView {
public:
    NodeView(NRef<T> base_node, bool first_time);
};

} // namespace lcs::ui
