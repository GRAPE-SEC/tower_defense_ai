// Copyright [2022] <Copyright Eita Aoki (Thunder) >
// 책에서 다룬 예제 게임 이외에도 시간 제약이 있는 빔 탐색을 하는데 사용할 수 있는 예제 코드 템플릿
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <queue>
#include <chrono>

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

std::mt19937 mt_for_action(0);                // 행동 선택용 난수 생성기 초기화
using ScoreType = int64_t;                    // 게임 평가 점수 자료형을 결정
constexpr const ScoreType INF = 1000000000LL; // 불가능한(무한) 점수의 예로 정의

class State
{
private:
public:
    ScoreType evaluated_score_ = 0; // 탐색을 통해 확인한 점수
    int first_action_ = -1;         // 탐색 트리의 루트 노드에서 처음으로 선택한 행동
    State() {}

    // 게임 종료 판정
    bool isDone() const
    {
        /*처리*/
        return /*(bool)종료 판정*/;
    }

    // 탐색용으로 게임판을 평가한다.
    void evaluateScore()
    {
        this->evaluated_score_ = /*(ScoreType) 평가 결과*/;
    }

    // 지정한 action으로 게임을 1턴 진행한다.
    void advance(const int action)
    {
        /*처리*/
    }

    // 현재 상황에서 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        /*처리*/
        return actions;
    }

    // // 필요에 따라 주석 해제
    // // 현재 게임 상황을 문자열로 만든다.
    // std::string toString() const
    // {
    //     return /*(std::string)문자열로 만든 게임 상황*/;
    // }
};

// 탐색할 때 정렬용으로 평가를 비교한다.
bool operator<(const State &state_1, const State &state_2)
{
    return state_1.evaluated_score_ < state_2.evaluated_score_;
}

// 빔 너비와 제한 시간(ms)을 지정해서 빔 탐색으로 행동을 결정한다.
int beamSearchActionWithTimeThreshold(
    const State &state,
    const int beam_width,
    const int64_t time_threshold)
{
    auto time_keeper = TimeKeeper(time_threshold);
    auto legal_actions = state.legalActions();
    std::priority_queue<State> now_beam;
    State best_state;

    now_beam.push(state);
    for (int t = 0;; t++)
    {
        std::priority_queue<State> next_beam;
        for (int i = 0; i < beam_width; i++)
        {
            if (time_keeper.isTimeOver())
            {
                return best_state.first_action_;
            }
            if (now_beam.empty())
                break;
            State now_state = now_beam.top();
            now_beam.pop();
            auto legal_actions = now_state.legalActions();
            for (const auto &action : legal_actions)
            {
                State next_state = now_state;
                next_state.advance(action);
                next_state.evaluateScore();
                if (t == 0)
                    next_state.first_action_ = action;
                next_beam.push(next_state);
            }
        }

        now_beam = next_beam;
        best_state = now_beam.top();

        if (best_state.isDone())
        {
            break;
        }
    }
    return best_state.first_action_;
}