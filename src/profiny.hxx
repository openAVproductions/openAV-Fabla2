/*
 * Profiny - Lightweight Profiler Tool
 * Copyright (C) 2013 Sercan Tutar
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * USAGE:
 *   First PROFINY_CALL_GRAPH_PROFILER or PROFINY_FLAT_PROFILER must be defined
 *   (giving as a compiler option is advised). If you;
 *
 *     - define PROFINY_CALL_GRAPH_PROFILER, it will work as a call-graph profiler
 *     - define PROFINY_FLAT_PROFILER, it will work as a flat profiler
 *     - define neither, Profiny macros will be set to blank (i.e. profiling will be off)
 *     - define both, it will give an error
 *
 *   Later, if you chose PROFINY_CALL_GRAPH_PROFILER, you may want to determine
 *   whether recursive calls will be omitted or not (omitted by default) by calling:
 *
 *     Profiler::setOmitRecursiveCalls(bool)
 *
 *   By default (if the profiling is not off), if your program exits normally, Profinity
 *   will print results in "profinity.out" file. Also, the user can force printing results
 *   at any time by calling:
 *
 *     Profiler::printStats("filename")
 *
 *   See ReadMe.txt for more info.
 *
 *
 * Happy profiling!
 *
 */


#ifndef PROFINY_H_
#define PROFINY_H_


#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <stdlib.h>


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


#if defined(PROFINY_CALL_GRAPH_PROFILER) && defined(PROFINY_FLAT_PROFILER)

#	error "PROFINY_CALL_GRAPH_PROFILER and PROFINY_FLAT_PROFILER should not be defined at the same time!"

#elif defined(PROFINY_CALL_GRAPH_PROFILER) || defined(PROFINY_FLAT_PROFILER)

#	define PROFINY_SCOPE \
		std::ostringstream _oss; \
		_oss << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__; \
		profiny::ScopedProfile _sco_pro(_oss.str());

#	define PROFINY_SCOPE_WITH_ID(ID) \
		std::ostringstream _oss; \
		_oss << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ":" << (ID); \
		profiny::ScopedProfile _sco_pro(_oss.str());

#	define PROFINY_NAMED_SCOPE(NAME) \
		std::ostringstream _oss; \
		_oss << (NAME); \
		profiny::ScopedProfile _sco_pro(_oss.str());

#	define PROFINY_NAMED_SCOPE_WITH_ID(NAME, ID) \
		std::ostringstream _oss; \
		_oss << (NAME) << ":" << (ID); \
		profiny::ScopedProfile _sco_pro(_oss.str());

#else

#	define PROFINY_SCOPE

#	define PROFINY_SCOPE_WITH_ID(ID)

#	define PROFINY_NAMED_SCOPE(NAME)

#	define PROFINY_NAMED_SCOPE_WITH_ID(NAME, ID)

#endif


#define NANOSEC_TO_SEC(X) ((X) / 1000000000.0)


namespace profiny
{
	class Timer
	{
	public:
		Timer();

		void start();

		void stop();

		double getElapsedTime();

	private:
		double m_startTime;

		double m_stopTime;

		bool m_running;

#ifdef _WIN32
		double m_reciprocalFrequency;
#endif

		double getTime();
	};

	/**********************************************************************/

	class BaseObject
	{
	public:
		BaseObject();

		virtual ~BaseObject();

		void incrRef();

		void decrRef();

		int getRef() const;

	private:
		int m_ref;
	};

	/**********************************************************************/

	class Profile : public BaseObject
	{
		friend class ScopedProfile;
		friend class Profiler;

	private:
		Profile(const std::string& name);

		~Profile();

		bool start();

		bool stop();

		unsigned int getCallCount() const;

		std::string getName() const;

		void getTimes(double& wall) const;

#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::map<std::string, Profile*>& getSubProfiles();

		std::map<std::string, Profile*> m_subProfiles;
#else
		bool m_timeStarted;
#endif

		std::string m_name;

		unsigned int m_callCount;

		double m_wallTime;

		Timer m_timer;
	};

	/**********************************************************************/

	class ScopedProfile : public BaseObject
	{
	public:
		ScopedProfile(const std::string& name);

		~ScopedProfile();

	private:
		Profile* m_profile;
	};

	/**********************************************************************/

	class Profiler : public BaseObject
	{
		friend class Profile;
		friend class ScopedProfile;

	public:
		static void printStats(const std::string& filename);

#ifdef PROFINY_CALL_GRAPH_PROFILER
		static void setOmitRecursiveCalls(bool omit);

		static bool getOmitRecursiveCalls();
#endif

	private:
		Profiler();

		~Profiler();

		static Profiler* getInstance();

		Profile* getProfile(const std::string& name);

		static void printStats();

		static void printStats(std::ofstream& fs, std::map<std::string, Profile*>* p, int depth);

#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::map<std::string, Profile*>& getCurrentProfilesRoot();

		void pushProfile(Profile* p);

		void popProfile();

		bool isInStack(const std::string& name);
#endif

		std::map<std::string, Profile*> m_profiles;

		static Profiler* m_instance;

#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::vector<Profile*> m_profileStack;

		bool m_omitRecursiveCalls;
#endif
	};

	/**********************************************************************/

	Timer::Timer()
		: m_startTime(0.0f), m_stopTime(0.0f), m_running(false)
	{
#ifdef _WIN32
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		m_reciprocalFrequency = 1.0f / freq.QuadPart;
#endif
	}

	void Timer::start()
	{
		m_running = true;
		m_startTime = getTime();
	}

	void Timer::stop()
	{
		m_running = false;
		m_stopTime = getTime() - m_startTime;
	}

	double Timer::getElapsedTime()
	{
		if (m_running)
			return getTime() - m_startTime;

		return m_stopTime;
	}

	double Timer::getTime()
	{
#ifdef _WIN32
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		double time = count.QuadPart * m_reciprocalFrequency;
#else
		struct timespec interval;
		clock_gettime(CLOCK_MONOTONIC, &interval);
		double time = interval.tv_sec + interval.tv_nsec * 0.000000001f;
#endif

		return time;
	};

	/**********************************************************************/

	inline BaseObject::BaseObject() :
			m_ref(0)
	{
	}

	inline BaseObject::~BaseObject()
	{
	}

	inline void BaseObject::incrRef()
	{
		++m_ref;
	}

	inline void BaseObject::decrRef()
	{
		--m_ref;
	}

	inline int BaseObject::getRef() const
	{
		return m_ref;
	}

	/**********************************************************************/

	inline Profile::Profile(const std::string& name) :
#ifndef PROFINY_CALL_GRAPH_PROFILER
			m_timeStarted(false),
#endif
			m_name(name), m_callCount(0), m_wallTime(0.0)
	{
	}

	inline Profile::~Profile()
	{
	}

	inline bool Profile::start()
	{
#ifdef PROFINY_CALL_GRAPH_PROFILER
		Profiler::getInstance()->pushProfile(this);
#else
		if (m_timeStarted)
		{
			return false;
		}
		m_timeStarted = true;
#endif
		m_timer.start();
		return true;
	}

	inline bool Profile::stop()
	{
#ifdef PROFINY_CALL_GRAPH_PROFILER
		Profiler::getInstance()->popProfile();
#else
		if (!m_timeStarted)
		{
			return false;
		}
		m_timeStarted = false;
#endif
		m_timer.stop(); // TODO: check if we need this line
		m_wallTime += m_timer.getElapsedTime();
		++m_callCount;
		return true;
	}

	inline unsigned int Profile::getCallCount() const
	{
		return m_callCount;
	}

	inline std::string Profile::getName() const
	{
		return m_name;
	}

	inline void Profile::getTimes(double& wall) const
	{
		wall = m_wallTime;
	}

#ifdef PROFINY_CALL_GRAPH_PROFILER
	inline std::map<std::string, Profile*>& Profile::getSubProfiles()
	{
		return m_subProfiles;
	}
#endif

	/**********************************************************************/

	inline ScopedProfile::ScopedProfile(const std::string& name) : m_profile(NULL)
	{
		std::string n(name);

#ifdef PROFINY_CALL_GRAPH_PROFILER
		if (Profiler::getInstance()->isInStack(n))
		{ // profile is already in stack (probably a recursive call)
			if (Profiler::getInstance()->getOmitRecursiveCalls())
			{
				return;
			}
			else
			{
				n = "RECURSIVE@" + n;
			}
		}
#endif

		m_profile = Profiler::getInstance()->getProfile(n);
		if (m_profile != NULL)
		{
			if (!m_profile->start())
			{ // cannot start profiler (probably a recursive call for flat profiler)
				delete m_profile;
				m_profile = NULL;
			}
		}
		else
		{
			std::cerr << "Cannot start scoped profiler: " << n << std::endl;
		}
	}

	inline ScopedProfile::~ScopedProfile()
	{
		if (m_profile != NULL)
		{
			m_profile->stop();
		}
	}

	/**********************************************************************/

	Profiler* Profiler::m_instance = NULL;

	inline Profiler::Profiler()
#ifdef PROFINY_CALL_GRAPH_PROFILER
		: m_omitRecursiveCalls(true)
#endif
	{
	}

	inline Profiler::~Profiler()
	{
	}

	inline Profiler* Profiler::getInstance()
	{
		if (m_instance == NULL)
		{
			m_instance = new Profiler;
			atexit(printStats);
		}
		return m_instance;
	}

	inline Profile* Profiler::getProfile(const std::string& name)
	{
#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::map<std::string, Profile*>& profiles = getCurrentProfilesRoot();
#else
		std::map<std::string, Profile*>& profiles = m_profiles;
#endif
		std::map<std::string, Profile*>::iterator it = profiles.find(name);
		if (it != profiles.end())
		{
			return it->second;
		}
		else
		{
			Profile* result = new Profile(name);
			profiles[name] = result;
			return result;
		}
	}

#ifdef PROFINY_CALL_GRAPH_PROFILER
	inline std::map<std::string, Profile*>& Profiler::getCurrentProfilesRoot()
	{
		return m_profileStack.empty() ? m_profiles : m_profileStack.back()->getSubProfiles();
	}

	inline void Profiler::pushProfile(Profile* p)
	{
		m_profileStack.push_back(p);
	}

	inline void Profiler::popProfile()
	{
		if (!m_profileStack.empty())
		{
			m_profileStack.pop_back();
		}
	}

	inline bool Profiler::isInStack(const std::string& name)
	{
		for (unsigned int i=0; i<m_profileStack.size(); ++i)
		{
			if (m_profileStack[i]->getName() == name)
			{
				return true;
			}
		}
		return false;
	}
#endif

	inline void Profiler::printStats(std::ofstream& fs, std::map<std::string, Profile*>* p, int depth)
	{
#ifdef PROFINY_CALL_GRAPH_PROFILER
		std::ostringstream oss;
		for (int i=0; i<depth; ++i)
		{
			oss << "\t";
		}
#endif

		std::map<std::string, Profile*>::iterator it = p->begin();
		for (; it != p->end(); ++it)
		{
			unsigned int cc = it->second->getCallCount();
			double wall;
			it->second->getTimes(wall);
#ifdef PROFINY_CALL_GRAPH_PROFILER
			fs << oss.str() << it->second->getName() << "  T:" << wall << "  #:" << cc << std::endl;
			printStats(fs, &(it->second->getSubProfiles()), depth+1);
#else
			fs << it->second->getName() << "  T:" << wall << "  #:" << cc << std::endl;
#endif
			delete it->second;
		}
	}

	inline void Profiler::printStats()
	{
		printStats("profiny.out");

		delete m_instance;
		m_instance = NULL;
	}

	inline void Profiler::printStats(const std::string& filename)
	{
		std::ofstream fs;
		fs.open(filename.c_str());
		if (!fs.is_open())
		{
			std::cerr << "Cannot open profiler output file: " << filename << std::endl;
			return;
		}
		Profiler::printStats(fs, &(getInstance()->m_profiles), 0);
		fs.close();
	}

#ifdef PROFINY_CALL_GRAPH_PROFILER
	inline void Profiler::setOmitRecursiveCalls(bool omit)
	{
		getInstance()->m_omitRecursiveCalls = omit;
	}

	inline bool Profiler::getOmitRecursiveCalls()
	{
		return getInstance()->m_omitRecursiveCalls;
	}
#endif

} // namespace profiny

/**********************************************************************/

inline void intrusive_ptr_add_ref(profiny::BaseObject* p)
{
    if (p != NULL)
    { // pointer is not NULL
        p->incrRef();
    }
}

inline void intrusive_ptr_release(profiny::BaseObject* p)
{
    if (p != NULL)
    { // pointer is not NULL
        p->decrRef();
        if (p->getRef() <= 0)
        { // reference count is zero or less
            delete p;
        }
    }
}

#endif /* PROFINY_H_ */
