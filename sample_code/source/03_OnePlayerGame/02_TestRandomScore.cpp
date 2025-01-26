// Copyright [2022] <Copyright Eita Aoki (Thunder) >
#include <string>
#include <sstream>
#include <random>
#include <iostream>
// 좌표를 저장하는 구조체
struct Coord
{
    int y_;
    int x_;
    Coord(const int y = 0, const int x = 0) : y_(y), x_(x) {}
};

std::mt19937 mt_for_action(0);                // 행동 선택용 난수 생성기 초기화
using ScoreType = int64_t;                    // 게임 평가 점수 자료형을 결정
constexpr const ScoreType INF = 1000000000LL; // 불가능한(무한) 점수의 예로 정의

constexpr const int H = 3;  // 미로의 높이
constexpr const int W = 4;  // 미로의 너비
constexpr int END_TURN = 4; // 게임 종료 턴

// 1인 게임 예
// 1턴에 상하좌우 네 방향 중 하나로 1칸 이동한다.
// 바닥에 있는 점수를 차지하면 자신의 점수가 되고, 바닥의 점수는 사라진다.
// END_TURN 시점에 높은 점수를 얻는 것이 목적
class MazeState
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0}; // 오른쪽, 왼쪽, 아래쪽, 위쪽으로 이동하는 이동방향 x축 값
    static constexpr const int dy[4] = {0, 0, 1, -1}; // 오른쪽, 왼쪽, 아래쪽, 위쪽으로 이동하는 이동방향 y축 값

    int points_[H][W] = {}; // 바닥의 점수는 1~9 중 하나
    int turn_ = 0;          // 현재 턴

public:
    Coord character_ = Coord();
    int game_score_ = 0;            // 게임에서 획득한 점수
    ScoreType evaluated_score_ = 0; // 탐색을 통해 확인한 점수
    MazeState() {}

    // h*w 크기의 미로를 생성한다.
    MazeState(const int seed)
    {
        auto mt_for_construct = std::mt19937(seed); // 게임판 구성용 난수 생성기 초기화
        this->character_.y_ = mt_for_construct() % H;
        this->character_.x_ = mt_for_construct() % W;

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                if (y == character_.y_ && x == character_.x_)
                {
                    continue;
                }
                this->points_[y][x] = mt_for_construct() % 10;
            }
    }

    // [모든 게임에서 구현] : 게임 종료 판정
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }
    // [모든 게임에서 구현] : 탐색용으로 게임판을 평가한다.
    void evaluateScore()
    {
        this->evaluated_score_ = this->game_score_; // 간단히 우선 기록 점수를 그대로 게임판의 평가로 사용
    }
    // [모든 게임에서 구현] : 지정한 action으로 게임을 1턴 진행한다.
    void advance(const int action)
    {
        this->character_.x_ += dx[action];
        this->character_.y_ += dy[action];
        auto &point = this->points_[this->character_.y_][this->character_.x_];
        if (point > 0)
        {
            this->game_score_ += point;
            point = 0;
        }
        this->turn_++;
    }

    // [모든 게임에서 구현] : 현재 상황에서 플레이어가 가능한 행동을 모두 획득한다.
    std::vector<int> legalActions() const
    {
        std::vector<int> actions;
        for (int action = 0; action < 4; action++)
        {
            int ty = this->character_.y_ + dy[action];
            int tx = this->character_.x_ + dx[action];
            if (ty >= 0 && ty < H && tx >= 0 && tx < W)
            {
                actions.emplace_back(action);
            }
        }
        return actions;
    }

    // [필수는 아니지만 구현하면 편리] : 현재 게임 상황을 문자열로 만든다.
    std::string toString() const
    {
        std::stringstream ss;
        ss << "turn:\t" << this->turn_ << "\n";
        ss << "score:\t" << this->game_score_ << "\n";
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                if (this->character_.y_ == h && this->character_.x_ == w)
                {
                    ss << '@';
                }
                else if (this->points_[h][w] > 0)
                {
                    ss << points_[h][w];
                }
                else
                {
                    ss << '.';
                }
            }
            ss << '\n';
        }

        return ss.str();
    }
};

using State = MazeState;

// 무작위로 행동을 결정한다.
int randomAction(const State &state)
{
    auto legal_actions = state.legalActions();
    return legal_actions[mt_for_action() % (legal_actions.size())];
}

// 탐욕법으로 행동을 결정한다.
int greedyAction(const State &state)
{
    auto legal_actions = state.legalActions();
    ScoreType best_score = -INF; // 불가능한만큼 작은 값으로 최고 점수를 초기화
    int best_action = -1;        // 불가능한 행동으로 초기화
    for (const auto action : legal_actions)
    {
        State now_state = state;
        now_state.advance(action);
        now_state.evaluateScore();
        if (now_state.evaluated_score_ > best_score)
        {
            best_score = now_state.evaluated_score_;
            best_action = action;
        }
    }
    return best_action;
}

// 게임을 game_number횟수만큼 플레이해서 평균 점수를 표시한다.
void testAiScore(const int game_number)
{
    using std::cout;
    using std::endl;
    std::mt19937 mt_for_construct(0);
    double score_mean = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto state = State(mt_for_construct());

        while (!state.isDone())
        {
            state.advance(randomAction(state));
        }
        auto score = state.game_score_;
        score_mean += score;
    }
    score_mean /= (double)game_number;
    cout << "Score:\t" << score_mean << endl;
}
int main()
{
    testAiScore(/*게임을 반복할 횟수*/ 100);
    return 0;
}