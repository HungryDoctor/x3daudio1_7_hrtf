#pragma once

#include <map>
#include <set>

template<typename T>
class EdgeRepository
{
public:

	void addEdge(T & predecessor, T & successor)
	{
		if (m_predecessorToSuccessors.find(predecessor) == m_predecessorToSuccessors.end())
		{
			m_predecessorToSuccessors.insert(std::make_pair(predecessor, std::set<T>()));
		}

		if (m_successorToPredecessors.find(successor) == m_successorToPredecessors.end())
		{
			m_successorToPredecessors.insert(std::make_pair(successor, std::set<T>()));
		}

		m_predecessorToSuccessors.at(predecessor).insert(successor);
		m_successorToPredecessors.at(successor).insert(predecessor);
	}

	void removeEdge(T & predecessor, T & successor)
	{
		{
			auto & record = m_predecessorToSuccessors.at(predecessor);
			record.erase(record.find(successor));
			if (record.size() == 0)
				m_predecessorToSuccessors.erase(m_predecessorToSuccessors.find(record));
		}
		{
			auto & record = m_successorToPredecessors.at(successor);
			record.erase(record.find(predecessor));
			if (record.size() == 0)
				m_successorToPredecessors.erase(m_successorToPredecessors.find(record));
		}
	}

	bool edgeExists(T & predecessor, T & successor) const
	{
		auto it = m_predecessorToSuccessors.at(predecessor);
		if (it == m_predecessorToSuccessors.end())
			return false;

		return it->find(predecessor) != it->end();
	}

	std::set<T> getSuccessors(T & vertex) const
	{
		auto it = m_predecessorToSuccessors.find(vertex);
		if (it == m_predecessorToSuccessors.end())
			return std::set<T>(0);
		else
			return *it;
	}

	std::set<T> getPredecessors(T & vertex) const
	{
		auto it = m_successorToPredecessors.find(vertex);
		if (it == m_successorToPredecessors.end())
			return std::set<T>(0);
		else
			return *it;
	}

private:
	std::map<T, std::set<T>> m_predecessorToSuccessors;
	std::map<T, std::set<T>> m_successorToPredecessors;
};