#include "parser.h"

std::unique_ptr<Lox::Expression> Lox::parse(const std::vector<Lox::Token>& tokens)
{
	uint32_t current = 0;

	auto peek = [&]() -> const Lox::Token&
	{
		return tokens.at(current);
	};

	auto is_at_end = [&]() -> bool
	{
		return peek().type == TokenType::EOF_;
	};

	auto previous = [&]() -> const Lox::Token&
	{
		return tokens.at(current - 1);
	};

	auto advance = [&]() -> const Lox::Token&
	{
		if (!is_at_end())
		{
			current++;
		}
		return previous();
	};

	auto check = [&](TokenType type) -> bool
	{
		if (is_at_end())
		{
			return false;
		}

		return peek().type == type;
	};

	auto match = [&](const std::vector<TokenType>& types)
	{
		for (TokenType type : types)
		{
			if (check(type))
			{
				advance();
				return true;
			}
		}

		return false;
	};

	auto parse_comparison = [&]() -> std::unique_ptr<Expression>
	{
		return nullptr;
	};

	auto parse_equality = [&]() -> std::unique_ptr<Expression>
	{
		std::unique_ptr<Expression> expr = parse_comparison();

		while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
		{
			const Lox::Token& op = previous();
			std::unique_ptr<Expression> right = parse_comparison();

			std::unique_ptr<BinaryExpression> new_expr = std::make_unique<BinaryExpression>();
			new_expr->left = std::move(expr);
			new_expr->op = op;
			new_expr->right = std::move(right);

			expr = std::move(new_expr);
		}

		return expr;
	};

	auto parse_expression = [&]() -> std::unique_ptr<Expression>
	{
		return parse_equality();
	};
}
