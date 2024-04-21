#include "parser.h"
#include "error.h"

#include <optional>

namespace ParserInternal
{
	using namespace Lox;

	class Parser
	{
	public:
		const std::vector<Token>& tokens;
		uint32_t current;

	public:
		Parser(const std::vector<Token>& in_tokens)
			: tokens(in_tokens)
			, current(0)
		{
		}

		std::vector<std::unique_ptr<Lox::Statement>> parse()
		{
			std::vector<std::unique_ptr<Lox::Statement>> statements;
			while (!is_at_end())
			{
				statements.push_back(parse_declaration());
			}

			return statements;
		}

	private:
		const Token& peek()
		{
			return tokens.at(current);
		}

		bool is_at_end()
		{
			return peek().type == TokenType::EOF_;
		};

		const Token& previous()
		{
			return tokens.at(current - 1);
		};

		const Token& advance()
		{
			if (!is_at_end())
			{
				current++;
			}
			return previous();
		};

		bool current_matches_type(TokenType type)
		{
			if (is_at_end())
			{
				return false;
			}

			return peek().type == type;
		};

		bool advance_for_token_types(const std::vector<TokenType>& types)
		{
			for (TokenType type : types)
			{
				if (current_matches_type(type))
				{
					advance();
					return true;
				}
			}

			return false;
		};

		ParseError create_error(const Token& token, const std::string& error_message)
		{
			Lox::report_error(token, error_message);
			return ParseError();
		}

		std::optional<const Token*> advance_for_token_type_checked(TokenType type, const std::string& error_message)
		{
			if (current_matches_type(type))
			{
				return {&advance()};
			}

			throw create_error(peek(), error_message);
		}

		void discard_tokens_until_next_statement()
		{
			advance();

			while (!is_at_end())
			{
				if (previous().type == TokenType::SEMICOLON)
				{
					return;
				}

				switch (peek().type)
				{
					case TokenType::CLASS:
					case TokenType::FUN:
					case TokenType::VAR:
					case TokenType::FOR:
					case TokenType::IF:
					case TokenType::WHILE:
					case TokenType::PRINT:
					case TokenType::RETURN:
					{
						return;
						break;
					}
					default:
					{
						break;
					}
				}

				advance();
			}
		}

		std::unique_ptr<Expression> parse_primary()
		{
			if (advance_for_token_types({TokenType::FALSE}))
			{
				std::unique_ptr<LiteralExpression> new_expr = std::make_unique<LiteralExpression>();
				new_expr->literal = false;
				return new_expr;
			}

			if (advance_for_token_types({TokenType::TRUE}))
			{
				std::unique_ptr<LiteralExpression> new_expr = std::make_unique<LiteralExpression>();
				new_expr->literal = true;
				return new_expr;
			}

			if (advance_for_token_types({TokenType::NIL}))
			{
				std::unique_ptr<LiteralExpression> new_expr = std::make_unique<LiteralExpression>();
				new_expr->literal = nullptr;
				return new_expr;
			}

			if (advance_for_token_types({TokenType::NUMBER, TokenType::STRING}))
			{
				std::unique_ptr<LiteralExpression> new_expr = std::make_unique<LiteralExpression>();
				new_expr->literal = previous().literal;
				return new_expr;
			}

			if (advance_for_token_types({TokenType::LEFT_PAREN}))
			{
				std::unique_ptr<GroupingExpression> new_expr = std::make_unique<GroupingExpression>();
				new_expr->expr = parse_expression();
				advance_for_token_type_checked(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
				return new_expr;
			}

			if (advance_for_token_types({TokenType::IDENTIFIER}))
			{
				std::unique_ptr<VariableExpression> new_expr = std::make_unique<VariableExpression>();
				new_expr->name = previous();
				return new_expr;
			}

			throw create_error(peek(), "Expected an expression.");
		}

		std::unique_ptr<Expression> finish_parsing_call(std::unique_ptr<Expression>&& callee)
		{
			std::vector<std::unique_ptr<Expression>> arguments;
			if (!current_matches_type(TokenType::RIGHT_PAREN))
			{
				do
				{
					if (arguments.size() > 255)
					{
						Lox::report_error(peek(), "Can't have more than 255 arguments");
					}

					arguments.push_back(parse_expression());
				} while (advance_for_token_types({TokenType::COMMA}));
			}

			const Token* paren = advance_for_token_type_checked(TokenType::RIGHT_PAREN, "Expected ')' after arguments").value();

			std::unique_ptr<CallExpression> new_expr = std::make_unique<CallExpression>();
			new_expr->callee = std::move(callee);
			new_expr->paren = *paren;
			new_expr->arguments = std::move(arguments);
			return new_expr;
		}

		std::unique_ptr<Expression> parse_call()
		{
			std::unique_ptr<Expression> expr = parse_primary();

			while (true)
			{
				if (advance_for_token_types({TokenType::LEFT_PAREN}))
				{
					expr = finish_parsing_call(std::move(expr));
				}
				else
				{
					break;
				}
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_unary()
		{
			if (advance_for_token_types({TokenType::BANG, TokenType::MINUS}))
			{
				std::unique_ptr<UnaryExpression> new_expr = std::make_unique<UnaryExpression>();
				new_expr->op = previous();
				new_expr->right = parse_unary();
				return new_expr;
			}

			return parse_call();
		}

		std::unique_ptr<Expression> parse_factor()
		{
			std::unique_ptr<Expression> expr = parse_unary();

			while (advance_for_token_types({TokenType::SLASH, TokenType::STAR}))
			{
				std::unique_ptr<BinaryExpression> new_expr = std::make_unique<BinaryExpression>();
				new_expr->left = std::move(expr);
				new_expr->op = previous();
				new_expr->right = parse_unary();

				expr = std::move(new_expr);
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_term()
		{
			std::unique_ptr<Expression> expr = parse_factor();

			while (advance_for_token_types({TokenType::MINUS, TokenType::PLUS}))
			{
				std::unique_ptr<BinaryExpression> new_expr = std::make_unique<BinaryExpression>();
				new_expr->left = std::move(expr);
				new_expr->op = previous();
				new_expr->right = parse_factor();

				expr = std::move(new_expr);
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_comparison()
		{
			std::unique_ptr<Expression> expr = parse_term();

			while (advance_for_token_types({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL}))
			{
				std::unique_ptr<BinaryExpression> new_expr = std::make_unique<BinaryExpression>();
				new_expr->left = std::move(expr);
				new_expr->op = previous();
				new_expr->right = parse_term();

				expr = std::move(new_expr);
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_equality()
		{
			std::unique_ptr<Expression> expr = parse_comparison();

			while (advance_for_token_types({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
			{
				std::unique_ptr<BinaryExpression> new_expr = std::make_unique<BinaryExpression>();
				new_expr->left = std::move(expr);
				new_expr->op = previous();
				new_expr->right = parse_comparison();

				expr = std::move(new_expr);
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_and()
		{
			std::unique_ptr<Expression> expr = parse_equality();

			while (advance_for_token_types({TokenType::AND}))
			{
				std::unique_ptr<LogicalExpression> new_expr = std::make_unique<LogicalExpression>();
				new_expr->left = std::move(expr);
				new_expr->op = previous();
				new_expr->right = parse_equality();

				expr = std::move(new_expr);
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_or()
		{
			std::unique_ptr<Expression> expr = parse_and();

			while (advance_for_token_types({TokenType::OR}))
			{
				std::unique_ptr<LogicalExpression> new_expr = std::make_unique<LogicalExpression>();
				new_expr->left = std::move(expr);
				new_expr->op = previous();
				new_expr->right = parse_and();

				expr = std::move(new_expr);
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_assignment()
		{
			std::unique_ptr<Expression> expr = parse_or();

			if (advance_for_token_types({TokenType::EQUAL}))
			{
				const Token& target = previous();
				std::unique_ptr<Expression> value = parse_assignment();

				// This is the only valid assignment target: Where the expr on the left is a variable expression
				if (VariableExpression* var_expr = dynamic_cast<VariableExpression*>(expr.get()))
				{
					std::unique_ptr<AssignmentExpression> new_expr = std::make_unique<AssignmentExpression>();
					new_expr->name = var_expr->name;
					new_expr->value = std::move(value);
					return new_expr;
				}

				Lox::report_error(target, "Invalid assignment target");
			}

			return expr;
		}

		std::unique_ptr<Expression> parse_expression()
		{
			return parse_assignment();
		}

		std::unique_ptr<Statement> parse_print_statement()
		{
			std::unique_ptr<Expression> expr = parse_expression();
			advance_for_token_type_checked(TokenType::SEMICOLON, "Expected a ';' after value.");

			std::unique_ptr<PrintStatement> statement = std::make_unique<PrintStatement>();
			statement->expression = std::move(expr);
			return statement;
		}

		std::unique_ptr<Statement> parse_expression_statement()
		{
			std::unique_ptr<Expression> expr = parse_expression();
			advance_for_token_type_checked(TokenType::SEMICOLON, "Expected a ';' after expression.");

			std::unique_ptr<ExpressionStatement> statement = std::make_unique<ExpressionStatement>();
			statement->expression = std::move(expr);
			return statement;
		}

		std::vector<std::unique_ptr<Statement>> parse_block()
		{
			std::vector<std::unique_ptr<Statement>> statements;

			while (!current_matches_type(TokenType::RIGHT_BRACE) && !is_at_end())
			{
				statements.push_back(parse_declaration());
			}

			advance_for_token_type_checked(TokenType::RIGHT_BRACE, "Expected a '}' after block.");
			return statements;
		}

		std::unique_ptr<Statement> parse_if_statement()
		{
			advance_for_token_type_checked(TokenType::LEFT_PAREN, "Expected a '(' after 'if'");
			std::unique_ptr<Expression> condition = parse_expression();
			advance_for_token_type_checked(TokenType::RIGHT_PAREN, "Expected a ')' after 'if' condition");

			std::unique_ptr<Statement> then_branch = parse_statement();

			std::unique_ptr<Statement> else_branch = nullptr;
			if (advance_for_token_types({TokenType::ELSE}))
			{
				else_branch = parse_statement();
			}

			std::unique_ptr<IfStatement> if_statement = std::make_unique<IfStatement>();
			if_statement->condition = std::move(condition);
			if_statement->then_branch = std::move(then_branch);
			if_statement->else_branch = std::move(else_branch);
			return if_statement;
		}

		std::unique_ptr<Statement> parse_while_statement()
		{
			advance_for_token_type_checked(TokenType::LEFT_PAREN, "Expected a '(' after 'while'");
			std::unique_ptr<Expression> condition = parse_expression();
			advance_for_token_type_checked(TokenType::RIGHT_PAREN, "Expected a ')' after 'while' condition");

			std::unique_ptr<Statement> body = parse_statement();

			std::unique_ptr<WhileStatement> while_statement = std::make_unique<WhileStatement>();
			while_statement->condition = std::move(condition);
			while_statement->body = std::move(body);
			return while_statement;
		}

		std::unique_ptr<Statement> parse_for_statement()
		{
			advance_for_token_type_checked(TokenType::LEFT_PAREN, "Expected a '(' after 'for'");

			// Parse initializer
			std::unique_ptr<Statement> initializer;
			if (advance_for_token_types({TokenType::SEMICOLON}))
			{
				initializer = nullptr;
			}
			else if (advance_for_token_types({TokenType::VAR}))
			{
				initializer = parse_var_declaration();
			}
			else
			{
				initializer = parse_expression_statement();
			}

			// Parse condition
			std::unique_ptr<Expression> condition = nullptr;
			if (!current_matches_type(TokenType::SEMICOLON))
			{
				condition = parse_expression();
			}
			advance_for_token_type_checked(TokenType::SEMICOLON, "Expected a ';' after 'for' condition");

			// Parse increment
			std::unique_ptr<Expression> increment = nullptr;
			if (!current_matches_type(TokenType::RIGHT_PAREN))
			{
				increment = parse_expression();
			}
			advance_for_token_type_checked(TokenType::RIGHT_PAREN, "Expected a ')' after 'for' loop clauses");

			// Parse body
			std::unique_ptr<Statement> body = parse_statement();

			// Replace body with a block that executes the previous body + the increment expression, if we have one
			if (increment != nullptr)
			{
				std::unique_ptr<ExpressionStatement> increment_statement = std::make_unique<ExpressionStatement>();
				increment_statement->expression = std::move(increment);

				std::unique_ptr<BlockStatement> new_body = std::make_unique<BlockStatement>();
				new_body->statements.push_back(std::move(body));
				new_body->statements.push_back(std::move(increment_statement));

				body = std::move(new_body);
			}

			// Replace condition with a 'true' if we don't have one
			if (condition == nullptr)
			{
				std::unique_ptr<LiteralExpression> new_condition = std::make_unique<LiteralExpression>();
				new_condition->literal = true;

				condition = std::move(new_condition);
			}

			// Replace body with a while loop that executes the previous actual body, checking the condition
			std::unique_ptr<WhileStatement> while_statement = std::make_unique<WhileStatement>();
			while_statement->condition = std::move(condition);
			while_statement->body = std::move(body);
			body = std::move(while_statement);

			// Replace body once again with a block that first executes the initializer, if we have one
			if (initializer != nullptr)
			{
				std::unique_ptr<BlockStatement> new_body = std::make_unique<BlockStatement>();
				new_body->statements.push_back(std::move(initializer));
				new_body->statements.push_back(std::move(body));

				body = std::move(new_body);
			}

			return body;
		}

		std::unique_ptr<Statement> parse_statement()
		{
			if (advance_for_token_types({TokenType::FOR}))
			{
				return parse_for_statement();
			}
			if (advance_for_token_types({TokenType::IF}))
			{
				return parse_if_statement();
			}
			if (advance_for_token_types({TokenType::PRINT}))
			{
				return parse_print_statement();
			}
			if (advance_for_token_types({TokenType::WHILE}))
			{
				return parse_while_statement();
			}
			if (advance_for_token_types({TokenType::LEFT_BRACE}))
			{
				std::unique_ptr<BlockStatement> block_statement = std::make_unique<BlockStatement>();
				block_statement->statements = parse_block();
				return block_statement;
			}

			return parse_expression_statement();
		}

		std::unique_ptr<Statement> parse_var_declaration()
		{
			std::optional<const Token*> name = advance_for_token_type_checked(TokenType::IDENTIFIER, "Expected a variable name");

			std::unique_ptr<Expression> initializer;
			if (advance_for_token_types({TokenType::EQUAL}))
			{
				initializer = parse_expression();
			}

			advance_for_token_type_checked(TokenType::SEMICOLON, "Expected a ';' after variable declaration");

			std::unique_ptr<VariableDeclarationStatement> statement = std::make_unique<VariableDeclarationStatement>();
			statement->name = *name.value();
			statement->initializer = std::move(initializer);
			return statement;
		}

		std::unique_ptr<Statement> parse_declaration()
		{
			try
			{
				if (advance_for_token_types({TokenType::VAR}))
				{
					return parse_var_declaration();
				}

				return parse_statement();
			}
			catch ([[maybe_unused]] const Lox::ParseError& e)
			{
				discard_tokens_until_next_statement();
				return {};
			}
		}
	};
};	  // namespace ParserInternal

std::vector<std::unique_ptr<Lox::Statement>> Lox::parse(const std::vector<Lox::Token>& tokens)
{
	ParserInternal::Parser parser{tokens};
	return parser.parse();
}
