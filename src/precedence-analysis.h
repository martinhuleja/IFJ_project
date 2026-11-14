/**
 * @file   precedence-analysis.h
 * @author Martin Huleja (xhulejm00)
 * @date   2026-11-14
 *
 * @brief  Header file for precedence analysis functions
 *
 * Precedence analysis creates AST subtree for expressions as bottom-up syntactic analysis.
 * Allocates subtree, links in to given ASTNode pointer as children and fills it with data.
 */

#ifndef PRECEDENCE_ANALYSIS_H
#define PRECEDENCE_ANALYSIS_H

#include "abstract-syntactic-tree.h"
#include "error.h"
#include "parser.h"

/********************************************************************
 ****************** PRECEDENCE ANALYSIS FUNCTIONS *******************
 *******************************************************************/

/**
 * @brief Parses expression by bottom-up syntactic analysis
 *
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving expression
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr(Parser *parser, ASTNode *parent);

/**
 * @brief Parses expression tail by bottom-up syntactic analysis
 *
 * @param parser Ptr to struct parser
 * @param parent Ptr to the parent node receiving expression
 * @return ErrorCode according to specification of IFJ26
 */
ErrorCode parse_expr_tail(Parser *parser, ASTNode *expression);

#endif // PRECEDENCE_ANALYSIS_H
/* End of file precedence-analysis.h */
