#include "ast/transform/ReifyEquivalences.h"
#include "ast/Atom.h"
#include "ast/Clause.h"
#include "ast/EqrelType.h"
#include "ast/IntrinsicFunctor.h"
#include "ast/PosetType.h"
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
    std::set<QualifiedName> posets;
    for (auto* type : program.getTypes()) {
        if (isA<ast::EqrelType>(type)) {
            // Create a relation for the type
            auto rel = mk<Relation>(type->getQualifiedName(), type->getSrcLoc());
            rel->setRepresentation(RelationRepresentation::EQREL);
            rel->addQualifier(RelationQualifier::TYPE);
            rel->addAttribute(mk<Attribute>("x", type->getQualifiedName()));
            rel->addAttribute(mk<Attribute>("y", type->getQualifiedName()));
            program.addRelation(std::move(rel));

            eqrels.insert(type->getQualifiedName());
            changed |= true;
        } else if (isA<ast::PosetType>(type)) {
            // Create a relation for the type's equivalence
            QualifiedName eqrelName = type->getQualifiedName();
            eqrelName.append("_eqrel");
            auto eqrel = mk<Relation>(eqrelName, type->getSrcLoc());
            eqrel->setRepresentation(RelationRepresentation::EQREL);
            eqrel->addQualifier(RelationQualifier::TYPE);
            eqrel->addAttribute(mk<Attribute>("x", type->getQualifiedName()));
            eqrel->addAttribute(mk<Attribute>("y", type->getQualifiedName()));
            program.addRelation(std::move(eqrel));

            // Create a relation for the type's partial order
            auto rel = mk<Relation>(type->getQualifiedName(), type->getSrcLoc());
            rel->setRepresentation(RelationRepresentation::DEFAULT);
            rel->addQualifier(RelationQualifier::TYPE);
            rel->addAttribute(mk<Attribute>("x", type->getQualifiedName()));
            rel->addAttribute(mk<Attribute>("y", type->getQualifiedName()));
            program.addRelation(std::move(rel));

            // Populate reflexive p.o. rules
            Own<ast::Atom> po = mk<ast::Atom>(type->getQualifiedName());
            po->addArgument(mk<ast::Variable>("x"));
            po->addArgument(mk<ast::Variable>("x"));
            Own<ast::Atom> eq = mk<ast::Atom>(eqrelName);
            eq->addArgument(mk<ast::Variable>("x"));
            eq->addArgument(mk<ast::Variable>("_x"));
            Own<Clause> insert = mk<Clause>(std::move(po));
            insert->addToBody(std::move(eq));
            program.addClause(std::move(insert));

            posets.insert(type->getQualifiedName());
            changed |= true;
        } else {
            continue;
        }
    }

    for (auto* rel : program.getRelations()) {
        // Don't create insertion or subsumption rules for equivalence
        // relations themselves.
        if (eqrels.find(rel->getQualifiedName()) != eqrels.end()) {
            continue;
        }
        if (posets.find(rel->getQualifiedName()) != posets.end()) {
            continue;
        }
        bool hasEquivalence = false;
        for (auto* attr : rel->getAttributes()) {
            if (eqrels.find(attr->getTypeName()) != eqrels.end()) {
                hasEquivalence = true;
                break;
            }
            if (posets.find(attr->getTypeName()) != posets.end()) {
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
                    // This argument has an equivalence relation
                    lt->addArgument(mk<ast::Variable>(attr->getName()));
                    Own<ast::IntrinsicFunctor> canon =
                            mk<ast::IntrinsicFunctor>("canonicalize", mk<ast::Variable>(attr->getName()));
                    gt->addArgument(std::move(canon));
                    Own<ast::Atom> elem = mk<ast::Atom>(attr->getTypeName());
                    elem->addArgument(mk<ast::Variable>(attr->getName()));
                    elem->addArgument(mk<ast::Variable>("_" + attr->getName()));
                    elems.push_back(std::move(elem));
                } else if (posets.find(attr->getTypeName()) != posets.end()) {
                    // This argument has a partial order
                    lt->addArgument(mk<ast::Variable>(attr->getName()));
                    Own<ast::IntrinsicFunctor> canon = mk<ast::IntrinsicFunctor>(
                            "canonicalize", mk<ast::Variable>(attr->getName() + "_above"));
                    gt->addArgument(std::move(canon));

                    Own<ast::Atom> elem = mk<ast::Atom>(attr->getTypeName());
                    elem->addArgument(mk<ast::Variable>(attr->getName()));
                    elem->addArgument(mk<ast::Variable>("_" + attr->getName()));
                    elems.push_back(std::move(elem));

                    Own<ast::Atom> leq = mk<ast::Atom>(attr->getTypeName());
                    leq->addArgument(mk<ast::Variable>(attr->getName()));
                    leq->addArgument(mk<ast::Variable>(attr->getName() + "_above"));
                    elems.push_back(std::move(leq));

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
