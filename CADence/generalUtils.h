#pragma once
#include <memory>
#include "Node.h"

std::weak_ptr<Node> FindObjectNode(std::vector<std::weak_ptr<Node>> nodesVector, Object& object);
std::shared_ptr<Node> FindObjectNode(std::vector<std::shared_ptr<Node>> nodesVector, Object& object);