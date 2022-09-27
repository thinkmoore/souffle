#include "ast/transform/ReifyEquivalences.h"
#include "ast/Atom.h"
#include "ast/Clause.h"
#include "ast/EqrelType.h"
#include "ast/IntrinsicFunctor.h"
#include "ast/Program.h"
#include "ast/Relation.h"
#include "ast/SubsumptiveClause.h"
#include "ast/Variable.h"
#include <set>
#include <vector>

namespace souffle::ast::transform {

bool ReifyEquivalencesTransformer::transform(TranslationUnit& translationUnit) {
    bool changed = false;
    Program& program = translationUnit.getProgram();

    std::set<QualifiedName> eqrels;
    for (auto* type : program.getTypes()) {
        if (!isA<ast::EqrelType>(type)) {
            continue;
        }
        // Create a relation for the type
        auto rel = mk<Relation>(type->getQualifiedName(), type->getSrcLoc());
        rel->setRepresentation(RelationRepresentation::EQREL_TYPE);
        rel->addAttribute(mk<Attribute>("x", type->getQualifiedName()));
        rel->addAttribute(mk<Attribute>("y", type->getQualifiedName()));
        program.addRelation(std::move(rel));

        eqrels.insert(type->getQualifiedName());
        changed |= true;
    }

    for (auto* rel : program.getRelations()) {
        // Don't create insertion or subsumption rules for equivalence
        // relations themselves.
        if (eqrels.find(rel->getQualifiedName()) != eqrels.end()) {
            continue;
        }
        bool hasEquivalence = false;
        for (auto* attr : rel->getAttributes()) {
            if (eqrels.find(attr->getTypeName()) != eqrels.end()) {
                hasEquivalence = true;
                break;
            }
        }
        if (hasEquivalence) {
            Own<ast::Atom> lt = mk<ast::Atom>(rel->getQualifiedName());
            Own<ast::Atom> gt = mk<ast::Atom>(rel->getQualifiedName());
            std::vector<Own<ast::Atom>> elems;
            for (auto* attr : rel->getAttributes()) {
                if (eqrels.find(attr->getTypeName()) != eqrels.end()) {
                    lt->addArgument(mk<ast::Variable>(attr->getName()));
                    Own<ast::IntrinsicFunctor> canon =
                            mk<ast::IntrinsicFunctor>("canonicalize", mk<ast::Variable>(attr->getName()));
                    gt->addArgument(std::move(canon));
                    Own<ast::Atom> elem = mk<ast::Atom>(attr->getTypeName());
                    elem->addArgument(mk<ast::Variable>(attr->getName()));
                    elem->addArgument(mk<ast::Variable>("_" + attr->getName()));
                    elems.push_back(std::move(elem));
                } else {
                    lt->addArgument(mk<ast::Variable>(attr->getName()));
                    gt->addArgument(mk<ast::Variable>(attr->getName()));
                }
            }
            Own<SubsumptiveClause> canonicalize = mk<SubsumptiveClause>(std::move(clone(lt)));
            canonicalize->addToBodyFront(std::move(clone(gt)));
            canonicalize->addToBodyFront(std::move(clone(lt)));
            Own<Clause> insert = mk<Clause>(std::move(gt));
            insert->addToBody(std::move(lt));
            auto elem = std::make_move_iterator(elems.begin());
            auto end = std::make_move_iterator(elems.end());
            for (; elem != end; elem++) {
                insert->addToBody(std::move(*elem));
            }
            program.addClause(std::move(insert));
            program.addClause(std::move(canonicalize));
        }
    }
    return changed;
}

}  // namespace souffle::ast::transform
