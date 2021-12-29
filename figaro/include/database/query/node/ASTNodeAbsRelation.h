#ifndef _AST_NODE_ABS_RELATION_H_
#define _AST_NODE_ABS_RELATION_H_

#include "utils/Utils.h"
#include "ASTNode.h"

namespace Figaro
{
    class ASTNodeRelation;

    class ASTNodeAbsRelation: public ASTNode
    {
        enum class SelectionCriterion
        {
            EXACT_VALUE, WHOLE_RANGE
        };
        struct SelectionAttribute
        {
            std::string m_attrName;
            SelectionCriterion m_criterion;
            double m_exactValue = 0.0;

            SelectionAttribute(const std::string& attrName,
                              SelectionCriterion criterion,
                              double exactValue = 0.0) : m_attrName(attrName),
                              m_criterion(criterion), m_exactValue(exactValue)
                              {}
        };
        // In initial version, in this variable will represent
        //  all the attributes from the child nodes of the current node.
        //std::vector<SelectionAttribute> m_selectAttributes;
        ASTNodeAbsRelation* m_pParent = nullptr;
        // TODO: Move to relation
    public:
        void setParent(ASTNodeAbsRelation* pParent)
        {
            m_pParent = pParent;
        }
        ASTNodeAbsRelation* getParent(void)
        {
            return m_pParent;
        }
        virtual ASTNodeRelation* getRelation(void) = 0;

        virtual const std::vector<std::string>& getAttributeNames(void) const = 0;

        virtual const std::vector<std::string>& getParJoinAttributeNames(void) = 0;

        /**
         * Returns the join attributes of the current node.
         */
        virtual const std::vector<std::string>& getJoinAttributeNames(void) = 0;

        /**
         * Intersects the attributes of the current node and its parrent and
         * children.
         */
        virtual void checkAndUpdateJoinAttributes(void) = 0;

        /**
         * Updates parent join attributes of the children for the current node.
         *
         * @pre Join attributes for this node need to be initialized.
         */

        virtual void updateParJoinAttrs(void) = 0;

        virtual ~ASTNodeAbsRelation() override = 0;

        virtual ASTNodeAbsRelation* copy() = 0;
    };

    inline ASTNodeAbsRelation::~ASTNodeAbsRelation() {}


}

#endif