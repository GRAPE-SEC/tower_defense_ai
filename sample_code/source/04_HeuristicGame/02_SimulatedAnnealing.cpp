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

std::mt19937 mt_for_action(0);

constexpr const int H = 5;     // 미로의 높이
constexpr const int W = 5;     // 미로의 너비
constexpr int END_TURN = 5;    // 게임 종료 턴
constexpr int CHARACTER_N = 3; // 캐릭터 개수

using ScoreType = int64_t;
constexpr const ScoreType INF = 1000000000LL;

// 좌표를 저장하는 구조체
struct Coord
{
    int y_;
    int x_;
    Coord(const int y = 0, const int x = 0) : y_(y), x_(x) {}
};

// 자동 1인 게임 예
// 캐릭터는 1칸 옆의 가장 점수가 높은 바닥으로 자동으로 이동한다.
// 경우의 수 중에서 점수가 같은 것이 있으면 오른쪽, 왼쪽, 아래쪽, 위쪽 순으로 우선적으로 선택한다.
// 1턴에 상하좌우 네 방향 중 어딘가로 벽이 없는 곳으로 1칸 이동한다.
// 바닥에 있는 점수를 차지하면 자신의 점수가 되고, 바닥의 점수는 사라진다.
// END_TURN 시점에 높은 점수를 얻는 것이 목적.
// 게임에 개입하는 요소로 초기 상태에 캐릭터를 어디에 배치하는가 선택 가능하다.
// 어떻게 캐릭터를 배치하면 최종 점수가 높아질지 생각하는 게임.
class AutoMoveMazeState
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0};
    static constexpr const int dy[4] = {0, 0, 1, -1};

    int points_[H][W] = {}; // 바닥의 점수는 1~9 중 하나
    int turn_;              // 현재 턴
    Coord characters_[CHARACTER_N] = {};

    // 지정 캐릭터를 이동시킨다.
    void movePlayer(const int character_id)
    {
        Coord &character = this->characters_[character_id];
        int best_point = -INF;
        int best_action_index = 0;
        for (int action = 0; action < 4; action++)
        {
            int ty = character.y_ + dy[action];
            int tx = character.x_ + dx[action];
            if (ty >= 0 && ty < H && tx >= 0 && tx < W)
            {
                auto point = this->points_[ty][tx];
                if (point > best_point)
                {
                    best_point = point;
                    best_action_index = action;
                }
            }
        }

        character.y_ += dy[best_action_index];
        character.x_ += dx[best_action_index];
    }

    // 게임을 1턴 진행한다.
    void advance()
    {
        for (int character_id = 0; character_id < CHARACTER_N; character_id++)
        {
            movePlayer(character_id);
        }
        for (auto &character : this->characters_)
        {
            auto &point = this->points_[character.y_][character.x_];
            this->game_score_ += point;
            point = 0;
        }
        ++this->turn_;
    }

public:
    int game_score_;            // 게임에서 획득한 점수
    ScoreType evaluated_score_; // 탐색을 통해 확인한 점수

    // h*w 크기의 미로를 생성한다.
    AutoMoveMazeState(const int seed) : turn_(0),
                                        game_score_(0),
                                        evaluated_score_(0)
    {

        auto mt_for_construct = std::mt19937(seed);
        for (int y = 0; y < H; y++)
        {
            for (int x = 0; x < W; x++)
            {
                points_[y][x] = mt_for_construct() % 9 + 1;
            }
        }
    }

    // 지정 위치에 지정 캐릭터를 배치한다.
    void setCharacter(const int character_id, const int y, const int x)
    {
        this->characters_[character_id].y_ = y;
        this->characters_[character_id].x_ = x;
    }

    // [모든 게임에서 구현] : 게임 종료 판정
    bool isDone() const
    {
        return this->turn_ == END_TURN;
    }

    // [필수는 아니지만 구현하면 편리] : 현재 게임 상황을 문자열로 만든다.
    std::string toString() const
    {
        std::stringstream ss;
        ss << "turn:\t" << this->turn_ << "\n";
        ss << "score:\t" << this->game_score_ << "\n";
        auto board_chars = std::vector<std::vector<char>>(H, std::vector<char>(W, '.'));
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                bool is_written = false; // 해당 좌표에 기록할 문자가 결정되었는지 여부

                for (const auto &character : this->characters_)
                {
                    if (character.y_ == h && character.x_ == w)
                    {
                        ss << "@";
                        is_written = true;
                        break;
                    }
                    board_chars[character.y_][character.x_] = '@';
                }
                if (!is_written)
                {
                    if (this->points_[h][w] > 0)
                    {
                        ss << points_[h][w];
                    }
                    else
                    {
                        ss << '.';
                    }
                }
            }
            ss << '\n';
        }

        return ss.str();
    }

    // [모든 게임에서 구현] : 기록 점수를 계산한다.(toString을 구현하지 않는다면 인수 is_print와 해당 처리가 불필요)
    ScoreType getScore(bool is_print = false) const
    {
        auto tmp_state = *this;
        // 캐릭터 위치에 있는 점수를 삭제한다.
        for (auto &character : this->characters_)
        {
            auto &point = tmp_state.points_[character.y_][character.x_];
            point = 0;
        }
        // 종료할 때까지 캐릭터 이동을 반복한다.
        while (!tmp_state.isDone())
        {
            tmp_state.advance();
            if (is_print)
                std::cout << tmp_state.toString() << std::endl;
        }
        return tmp_state.game_score_;
    }

    // [모든 게임에서 구현] : 초기화한다.
    void init()
    {
        for (auto &character : this->characters_)
        {
            character.y_ = mt_for_action() % H;
            character.x_ = mt_for_action() % W;
        }
    }

    // [모든 게임에서 구현] : 상태 전이를 한다.
    void transition()
    {
        auto &character = this->characters_[mt_for_action() % CHARACTER_N];
        character.y_ = mt_for_action() % H;
        character.x_ = mt_for_action() % W;
    }
};

using State = AutoMoveMazeState;

State hillClimb(const State &state, int number)
{
    State now_state = state;
    now_state.init();
    ScoreType best_score = now_state.getScore();
    for (int i = 0; i < number; i++)
    {
        auto next_state = now_state;
        next_state.transition();
        auto next_score = next_state.getScore();
        if (next_score > best_score)
        {
            best_score = next_score;
            now_state = next_state;
        }
    }
    return now_state;
}

State simulatedAnnealing(const State &state, int number, double start_temp, double end_temp)
{
    State now_state = state;
    now_state.init();
    ScoreType best_score = now_state.getScore();
    ScoreType now_score = best_score;
    auto best_state = now_state;

    for (int i = 0; i < number; i++)
    {
        auto next_state = now_state;
        next_state.transition();
        auto next_score = next_state.getScore();
        double temp = start_temp + (end_temp - start_temp) * (i / number);
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
    }
    return best_state;
}

using AIFunction = std::function<State(const State &)>;

using StringAIPair = std::pair<std::string, AIFunction>;

// 게임을 game_number횟수만큼 플레이해서 평균 점수를 표시한다.
void testAiScore(const StringAIPair &ai, const int game_number)
{
    using std::cout;
    using std::endl;
    std::mt19937 mt_for_construct(0);
    double score_mean = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto state = State(mt_for_construct());
        state = ai.second(state);

        auto score = state.getScore(false);
        score_mean += score;
    }
    score_mean /= (double)game_number;
    cout << "Score of " << ai.first << ":\t" << score_mean << endl;
}

int main()
{
    int simulate_number = 10000;
    const std::vector<StringAIPair> ais = {StringAIPair("hillClimb", [&](const State &state)
                                                        { return hillClimb(state, simulate_number); }),
                                           StringAIPair("simulatedAnnealing", [&](const State &state)
                                                        { return simulatedAnnealing(state, simulate_number, /*start_temp*/ 500, /*end_temp*/ 10); })};
    int game_number = 1000;
    for (const auto &ai : ais)
    {
        testAiScore(ai, game_number);
    }
    return 0;
}