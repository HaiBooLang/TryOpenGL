#pragma once

#include <chrono>

namespace Cactus
{
	class CactusEngine {
		
		static const float s_fps_alpha;

	public:
		void startEngine(const std::string& config_file_path);
		void shutdownEngine();

		void initialize();
		void clear();

		bool isQuit() const { return m_is_quit; }
		void run();
		bool tickOneFrame(float delta_time);

		int getFPS() const { return m_fps; }

		float calculateDeltaTime();

	protected:
		void logicalTick(float delta_time);
		bool rendererTick(float delta_time);

		void calculateFPS(float delta_time);

	protected:
		bool m_is_quit{ false };

		std::chrono::steady_clock::time_point m_last_tick_time_point{ std::chrono::steady_clock::now() };

		float m_average_duration{ 0.f };
		int   m_frame_count{ 0 };
		int   m_fps{ 0 };
	};
}

namespace Cactus
{
	float CactusEngine::calculateDeltaTime()
	{
		float delta_time;
		{
			using namespace std::chrono;

			steady_clock::time_point tick_time_point = steady_clock::now();
			duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_last_tick_time_point);
			delta_time = time_span.count();

			m_last_tick_time_point = tick_time_point;
		}
		return delta_time;
	}

	void CactusEngine::calculateFPS(float delta_time)
	{
		m_frame_count++;

		if (m_frame_count == 1)
		{
			m_average_duration = delta_time;
		}
		else
		{
			m_average_duration = m_average_duration * (1 - s_fps_alpha) + delta_time * s_fps_alpha;
		}

		m_fps = static_cast<int>(1.f / m_average_duration);
	}
}
