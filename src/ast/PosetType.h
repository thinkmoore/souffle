/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2021, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file PosetType.h
 *
 * Defines the poset type class
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
 * @class PosetType
 * @brief The poset type class
 *
 * Example:
 *  .type A = poset B
 *
 * An poset type associates a given type with an equivalence
 * relation over that type.
 */
class PosetType : public Type {
public:
    PosetType(QualifiedName name, QualifiedName aliasTypeName, SrcLocation loc = {});

    /** Return alias type */
    const QualifiedName& getPosetType() const {
        return aliasType;
    }

    /** Set alias type */
    void setPosetType(const QualifiedName& type) {
        aliasType = type;
    }

protected:
    void print(std::ostream& os) const override;

private:
    bool equal(const Node& node) const override;

    PosetType* cloning() const override;

private:
    /** Base type */
    QualifiedName aliasType;
};

}  // namespace souffle::ast
