// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <utility>
#include <random>
#include <assert.h>
#include <math.h>
#include <chrono>
#include <queue>
#include <algorithm>
#include <iostream>
#include <functional>

// Double형으로 경과 시간을 관리하는 클래스
class TimeKeeperDouble
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    double time_threshold_;

    double now_time_ = 0;

public:
    // 시간 제한을 밀리초 단위로 지정해서 인스턴스를 생성한다.
    TimeKeeperDouble(const double time_threshold)
        : start_time_(std::chrono::high_resolution_clock::now()),
          time_threshold_(time_threshold)
    {
    }

    // 경과 시간을 now_time_에 저장한다.
    void setNowTime()
    {
        auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
        this->now_time_ = std::chrono::duration_cast<std::chrono::microseconds>(diff).count() * 1e-3; // ms
    }

    // 경과 시간 now_time_을 획득한다.
    double getNowTime() const
    {
        return this->now_time_;
    }

    // 인스턴스를 생성한 시점부터 지정한 시간 제한을 초과하지 않았는지 판정한다.
    bool isTimeOver() const
    {
        return now_time_ >= time_threshold_;
    }
};

std::mt19937 mt_for_action(0);

constexpr const int H = 5;     // 미로의 높이
constexpr const int W = 5;     // 미로의 너비
constexpr int END_TURN = 5;    // 게임 종료 턴
constexpr int CHARACTER_N = 3; // 캐릭터 개수

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

class State
{
private:
public:
    // 기록 점수를 계산한다.
    ScoreType getScore() const
    {

        return /*(ScoreType)평가 결과*/;
    }

    // 초기화한다.
    void init()
    {
        /*처리*/
    }

    // 상태 전이를 한다.
    void transition()
    {
        /*처리*/
    }

    // // 필요에 따라 주석 해제
    // // 현재 게임 상황을 문자열로 만든다.
    // std::string toString() const
    // {
    //     return /*(std::string)문자열로 만든 게임 상황*/;
    // }
};

State simulatedAnnealingWithTimeThreshold(const State &state, int64_t time_threshold, double start_temp, double end_temp)
{
    auto time_keeper = TimeKeeperDouble(time_threshold);
    State now_state = state;
    now_state.init();
    ScoreType best_score = now_state.getScore();
    ScoreType now_score = best_score;
    auto best_state = now_state;

    for (;;)
    {
        auto next_state = now_state;
        next_state.transition();
        auto next_score = next_state.getScore();
        double temp = start_temp + (end_temp - start_temp) * (time_keeper.getNowTime() / time_threshold);
        double probability = exp((next_score - now_score) / temp); // 확률 prob로 전이한다.
        bool is_force_next = probability > (mt_for_action() % INF) / (double)INF;
        if (next_score > now_score || is_force_next)
        {
            now_score = next_score;
            now_state = next_state;
        }

        if (next_score > best_score)
        {
            best_score = next_score;
            best_state = next_state;
        }
        time_keeper.setNowTime();
        if (time_keeper.isTimeOver())
        {
            break;
        }
    }
    return best_state;
}