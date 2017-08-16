#pragma once

#include <deque>

namespace CasiaBot
{
	namespace ActionHelper
	{
		template<typename T>
		bool IsDequeAllPositive(std::deque<T> &queue)
		{
			for (size_t i = 0; i < queue.size(); i++)
			{
				if (queue[i] <= 0)
				{
					return false;
				}
			}
			return true;
		}

		template<typename T>
		bool IsDequeNoneNegative(std::deque<T> &queue)
		{
			for (size_t i = 0; i < queue.size(); i++)
			{
				if (queue[i] < 0)
				{
					return false;
				}
			}
			return true;
		}

		template<typename T>
		bool IsDequeAllNegative(std::deque<T> &queue)
		{
			for (size_t i = 0; i < queue.size(); i++)
			{
				if (queue[i] > 0)
				{
					return false;
				}
			}
			return true;
		}
	}
}