/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2021 The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#include "ast/PosetType.h"
#include "souffle/utility/DynamicCasting.h"
#include <ostream>
#include <utility>

namespace souffle::ast {

PosetType::PosetType(QualifiedName name, QualifiedName aliasTypeName, SrcLocation loc)
        : Type(std::move(name), std::move(loc)), aliasType(std::move(aliasTypeName)) {}

void PosetType::print(std::ostream& os) const {
    os << ".type " << getQualifiedName() << " = eqrel " << getPosetType();
}

bool PosetType::equal(const Node& node) const {
    const auto& other = asAssert<PosetType>(node);
    return getQualifiedName() == other.getQualifiedName() && aliasType == other.aliasType;
}

PosetType* PosetType::cloning() const {
    return new PosetType(getQualifiedName(), getPosetType(), getSrcLoc());
}

}  // namespace souffle::ast
