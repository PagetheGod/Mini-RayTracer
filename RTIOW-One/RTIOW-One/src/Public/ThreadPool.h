#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <future>
#include <memory>

//A simple but flexible thread pool I got online
class VThreadPool
{
public:
	VThreadPool(size_t NumThreads = 0, bool IsUsingCustomThreadCount = false);
	~VThreadPool();

	/*
	* This part is where the flexibility comes in. Use a template coupled with future so we can query the result of an async task
	* Note: 1. Invoke_result deduces the return type of a function(given its arguments) at compile time
	*/
	template<typename Func, class... Args>
	auto SubmitTask(Func&& InFunc, Args&&... InArgs) -> std::future<typename std::invoke_result<Func, Args...>::type>;
private:
	std::vector<std::thread> m_Workers;
	std::queue<std::function<void()>> m_Tasks;
	std::mutex m_QueueMutex;
	std::condition_variable m_Condition;
	bool m_HasStopped = false;
};

template<typename Func, class ...Args>
inline auto VThreadPool::SubmitTask(Func&& InFunc, Args && ...InArgs) -> std::future<typename std::invoke_result<Func, Args ...>::type>
{
	//The majority of the function logics are about using std::future, so we can query it later
	using ReturnType = typename std::invoke_result<Func, Args ...>::type;
	/*
	* 1. std::packaged_task wraps a callable element and allows its result to be retrieved asynchronously, it passes its resualt to a std::future object
	* 2. std::bind creates a new callable object using the given function and arguments. It basically creates a lambda that captures the function and its arguments, this lambda receive no arguments
	* 3. Packaged task is move-only, so we use a shared pointer to pass it to the std::function and to the queue, while also keeping it alive until it is executed
	*/
	auto Task = std::make_shared<std::packaged_task<ReturnType()>>(
		std::bind(std::forward<Func>(InFunc), std::forward<Args>(InArgs)...)
	);

	std::future<ReturnType> Result = Task->get_future();
	{
		std::unique_lock<std::mutex> Lock(m_QueueMutex);
		if (m_HasStopped)
		{
			throw std::runtime_error("Exception: Submitting to a stopped Thread Pool!");
		}
		m_Tasks.emplace([Task]()
		{
			(*Task)();
		});
	}
	m_Condition.notify_one();
	return Result;
}
