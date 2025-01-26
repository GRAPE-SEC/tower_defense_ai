// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <random>
#include <math.h>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <queue>
#include <set>

// 시간을 관리하는 클래스
class TimeKeeper
{
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    int64_t time_threshold_;

public:
    // 시간 제한을 밀리초 단위로 지정해서 인스턴스를 생성한다.
    TimeKeeper(const int64_t &time_threshold)
        : start_time_(std::chrono::high_resolution_clock::now()),
          time_threshold_(time_threshold)
    {
    }

    // 인스턴스를 생성한 시점부터 지정한 시간 제한을 초과하지 않았는지 판정한다.
    bool isTimeOver() const
    {
        auto diff = std::chrono::high_resolution_clock::now() - this->start_time_;
        return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() >= time_threshold_;
    }
};

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

enum WinningStatus
{
    WIN,
    LOSE,
    DRAW,
    NONE,
};

class State
{
private:
public:
    // 게임 종료 판정
    bool isDone() const
    {
        /*처리*/
        return /*(bool)종료 판정*/;
    }

    // 지정한 action으로 게임을 1턴 진행하고 다음 플레이어 시점의 게임판으로 만든다.
    void advance(const int action)
    {
        /*처리*/
    }

    // 현재 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        /*처리*/
        return actions;
    }

    // 승패 정보를 획득한다.
    WinningStatus getWinningStatus() const
    {
        return /*(WinningStatus)승패*/;
    }

    // 현재 플레이어 시점에서 게임판을 평가한다.
    ScoreType getScore() const
    {
        return /*(ScoreType)현재 플레이어 시점의 게임판 평가값*/;
    }

    // // 필요에 따라 주석 해제
    // // 현재 게임 상황을 문자열로 만든다.
    // std::string toString() const
    // {
    //     return /*(std::string)문자열로 만든 게임 상황*/;
    // }
};

namespace iterativedeepening
{
    // 제한 시간이 넘으면 정지하는 알파-베타 가지치기용 점수 계산
    ScoreType alphaBetaScore(const State &state, ScoreType alpha, const ScoreType beta, const int depth, const TimeKeeper &time_keeper)
    {
        if (time_keeper.isTimeOver())
            return 0;
        if (state.isDone() || depth == 0)
        {
            return state.getScore();
        }
        auto legal_actions = state.legalActions();
        if (legal_actions.empty())
        {
            return state.getScore();
        }
        for (const auto action : legal_actions)
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -alphaBetaScore(next_state, -beta, -alpha, depth - 1, time_keeper);
            if (time_keeper.isTimeOver())
                return 0;
            if (score > alpha)
            {
                alpha = score;
            }
            if (alpha >= beta)
            {
                return alpha;
            }
        }
        return alpha;
    }
    // 깊이와 제한 시간(밀리초)을 지정해서 알파-베타 가지치기로 행동을 결정한다.
    int alphaBetaActionWithTimeThreshold(const State &state, const int depth, const TimeKeeper &time_keeper)
    {
        ScoreType best_action = -1;
        ScoreType alpha = -INF;
        for (const auto action : state.legalActions())
        {
            State next_state = state;
            next_state.advance(action);
            ScoreType score = -alphaBetaScore(next_state, -INF, -alpha, depth, time_keeper);
            if (time_keeper.isTimeOver())
                return 0;
            if (score > alpha)
            {
                best_action = action;
                alpha = score;
            }
        }
        return best_action;
    }

    // 제한 시간(밀리초)을 지정해서 반복 심화 탐색으로 행동을 결정한다.
    int iterativeDeepeningAction(const State &state, const int64_t time_threshold)
    {
        auto time_keeper = TimeKeeper(time_threshold);
        int best_action = -1;
        for (int depth = 1;; depth++)
        {
            int action = alphaBetaActionWithTimeThreshold(state, depth, time_keeper);

            if (time_keeper.isTimeOver())
            {
                break;
            }
            else
            {
                best_action = action;
            }
        }
        return best_action;
    }
}