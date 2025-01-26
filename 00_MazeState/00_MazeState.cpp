
struct Coord
{
    int y_;
    int x_;
    Coord(const int y = 0, const int x = 0) : y_(y), x_(x) {}
};

constexpr const int H = 3;
constexpr const int W = 4;
constexpr int END_TURN = 4;

class MazeState // 싱글톤패턴. 게임 전체를 관리하는 신적인 존재
{
    private:
        int points_[H][W] = {};
        int turn_ = 0;

    public:
    Coord character_ = Coord();
    int game_score_ = 0;
    MazeState() {}

    // h*w 크기의 미로를 생성한다.
    MazeState(const int seed)
    {
        auto mt_for_construct = std::mt19937(seed); // 게임판 구성용 난수 생성기 초기화
        this->character_.y_ = mt_for_constructor() % H;
        this->character_.x_ = mt_for_constructor() % W;

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
    
    bool isDone() const
    {
        return this->turn_ = END_TURN;
    }
}
