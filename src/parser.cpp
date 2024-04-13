#include "parser.h"
#include "error.h"

#include <optional>

namespace ParserInternal
{
	using namespace Lox;

	class ParseException
	{
	};

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

		std::unique_ptr<Lox::Expression> parse()
		{
			try
			{
				return parse_expression();
			}
			catch (const ParseException& e)
			{
				return nullptr;
			}
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

		ParseException create_error(const Token& token, const std::string& error_message)
		{
			Lox::report_error(token, error_message);
			return ParseException();
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

			throw create_error(peek(), "Expected an expression.");

			return nullptr;
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

			return parse_primary();
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

			while (advance_for_token_types({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS_EQUAL}))
			{
				std::unique_ptr<BinaryExpression> new_expr = std::make_unique<BinaryExpression>();
				new_expr->left = std::move(expr);
				new_expr->op = previous();
				new_expr->right = parse_term();

				expr = std::move(new_expr);
			}

			return expr;
		};

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
		};

		std::unique_ptr<Expression> parse_expression()
		{
			return parse_equality();
		};
	};
};	  // namespace ParserInternal

std::unique_ptr<Lox::Expression> Lox::parse(const std::vector<Lox::Token>& tokens)
{
	ParserInternal::Parser parser{tokens};
	return parser.parse();
}