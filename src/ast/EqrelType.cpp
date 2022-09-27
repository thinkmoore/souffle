/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2021 The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#include "ast/EqrelType.h"
#include "souffle/utility/DynamicCasting.h"
#include <ostream>
#include <utility>

namespace souffle::ast {

EqrelType::EqrelType(QualifiedName name, QualifiedName aliasTypeName, SrcLocation loc)
        : Type(std::move(name), std::move(loc)), aliasType(std::move(aliasTypeName)) {}

void EqrelType::print(std::ostream& os) const {
    os << ".type " << getQualifiedName() << " = eqrel " << getEqrelType();
}

bool EqrelType::equal(const Node& node) const {
    const auto& other = asAssert<EqrelType>(node);
    return getQualifiedName() == other.getQualifiedName() && aliasType == other.aliasType;
}

EqrelType* EqrelType::cloning() const {
    return new EqrelType(getQualifiedName(), getEqrelType(), getSrcLoc());
}

}  // namespace souffle::ast
