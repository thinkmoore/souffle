/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2021, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file EqrelType.h
 *
 * Defines the eqrel type class
 *
 ***********************************************************************/

#pragma once

#include "ast/QualifiedName.h"
#include "ast/Type.h"
#include "parser/SrcLocation.h"
#include <cstddef>
#include <iosfwd>

namespace souffle::ast {

/**
 * @class EqrelType
 * @brief The eqrel type class
 *
 * Example:
 *  .type A = eqrel B
 *
 * An eqrel type associates a given type with an equivalence
 * relation over that type.
 */
class EqrelType : public Type {
public:
    EqrelType(QualifiedName name, QualifiedName aliasTypeName, SrcLocation loc = {});

    /** Return alias type */
    const QualifiedName& getEqrelType() const {
        return aliasType;
    }

    /** Set alias type */
    void setEqrelType(const QualifiedName& type) {
        aliasType = type;
    }

protected:
    void print(std::ostream& os) const override;

private:
    bool equal(const Node& node) const override;

    EqrelType* cloning() const override;

private:
    /** Base type */
    QualifiedName aliasType;
};

}  // namespace souffle::ast
