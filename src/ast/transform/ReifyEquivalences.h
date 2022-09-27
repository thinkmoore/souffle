/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2020, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ReifyEquivalences.h
 *
 * Transformation pass to instantiate relations and rules associated
 * with eqrel types.
 *
 ***********************************************************************/

#pragma once

#include "ast/TranslationUnit.h"
#include "ast/transform/Transformer.h"
#include <string>

namespace souffle::ast::transform {

class ReifyEquivalencesTransformer : public Transformer {
public:
    std::string getName() const override {
        return "ReifyEquivalencesTransformer";
    }

private:
    ReifyEquivalencesTransformer* cloning() const override {
        return new ReifyEquivalencesTransformer();
    }

    bool transform(TranslationUnit& translationUnit) override;
};

}  // namespace souffle::ast::transform
