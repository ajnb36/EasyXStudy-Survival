#include <cmath>
#include <graphics.h>
#include <string>
#include <vector>
#include <iostream>

const int WINDOW_WIDTH = GetSystemMetrics(SM_CXFULLSCREEN) * 4 / 5;
const int WINDOW_HEIGHT = GetSystemMetrics(SM_CYFULLSCREEN) * 4 / 5;
const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

bool is_game_running = false;
bool isRunning = true;

void putimage_alpha(int x, int y, IMAGE &img) {
    int w = img.getwidth();
    int h = img.getheight();
    AlphaBlend(GetImageHDC(NULL), x, y, w, h,
               GetImageHDC(&img), 0, 0, w, h, {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA});
}

class Atlas {
public:
    Atlas(std::string path, int num) {
        TCHAR file_path[MAX_PATH];
        for (int i = 0; i < num; i++) {
            _stprintf_s(file_path, path.c_str(), i);
            IMAGE *img = new IMAGE();
            loadimage(img, file_path);
            frame_list.push_back(img);
        }
    }
    Atlas(std::string path, int num, Atlas* right) {
        TCHAR file_path[MAX_PATH];
        for (int i = 0; i < num; i++) {
            _stprintf_s(file_path, path.c_str(), i);
            IMAGE *img = new IMAGE();
            loadimage(img, file_path);
            frame_list.push_back(img);

        }
    }
    Atlas(){}

    std::vector<IMAGE *> getFrameList() const {
        return frame_list;
    }

    ~Atlas() {
        for (int i = 0; i < frame_list.size(); i++) {
            delete frame_list[i];
        }
    }

private:
    std::vector<IMAGE *> frame_list;
};

Atlas *player_left;
Atlas *player_right;
Atlas *enemy_left;
Atlas *enemy_right;

class Animation {
public:
    Animation(Atlas *anim_atlas, int interval) : anim_atlas(anim_atlas), interval_ms(interval) {}

    void Play(int x, int y, int delta) {
        timer += delta;
        if (timer >= interval_ms) {
            idx_frame = (idx_frame + 1) % (int) anim_atlas->getFrameList().size();
            timer = 0;
        }
        putimage_alpha(x, y, *(anim_atlas->getFrameList())[idx_frame]);
    }

    ~Animation() = default;

private:
    int timer = 0; // 动画计时器
    int interval_ms = 0; // 间隔时间 单位ms
    int idx_frame = 0; // 动画帧索引
    Atlas *anim_atlas;

};

class Player {
public:
    Player() {
        anim_left = new Animation(player_left, 45);
        anim_right = new Animation(player_right, 45);
        loadimage(&shadow_player, TEXT("./assert/img/shadow_player.png"));
    }

    void ProcessEvent(const ExMessage &msg) {
        if (msg.message == WM_KEYDOWN) {
            switch (msg.vkcode) {
                case VK_LEFT:
                    is_left = true;
                    break;
                case VK_RIGHT:
                    is_right = true;
                    break;
                case VK_UP:
                    is_up = true;
                    break;
                case VK_DOWN:
                    is_down = true;
                    break;
                default:;
            }
        } else if (msg.message == WM_KEYUP) {
            switch (msg.vkcode) {
                case VK_LEFT:
                    is_left = false;
                    break;
                case VK_RIGHT:
                    is_right = false;
                    break;
                case VK_UP:
                    is_up = false;
                    break;
                case VK_DOWN:
                    is_down = false;
                    break;
                default:;
            }
        }
    }

    void Move() {
        int dir_x = is_right - is_left;
        int dir_y = is_down - is_up;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len_dir != 0) {
            double normalized_x = player_speed * dir_x / len_dir;
            double normalized_y = player_speed * dir_y / len_dir;
            player_pos.x += (int) normalized_x;
            player_pos.y += (int) normalized_y;
        }

        // 保证玩家不出界
        if (player_pos.x < 0) player_pos.x = 0;
        if (player_pos.y < 0) player_pos.y = 0;
        if (player_pos.x + PLAY_WIDTH > WINDOW_WIDTH) player_pos.x = WINDOW_WIDTH - PLAY_WIDTH;
        if (player_pos.y + PLAY_HEIGHT > WINDOW_HEIGHT) player_pos.y = WINDOW_HEIGHT - PLAY_HEIGHT;
    }

    void Draw(int delta) {
        putimage_alpha(player_pos.x + PLAY_WIDTH / 2 - SHADOW_WIDTH / 2,
                       player_pos.y + PLAY_HEIGHT - shadow_player.getheight() / 2, shadow_player);

        static bool facing_left = false;
        int dir_x = is_right - is_left;
        if (dir_x > 0)
            facing_left = false;
        else if (dir_x < 0)
            facing_left = true;
        if (facing_left)
            anim_left->Play(player_pos.x, player_pos.y, delta);
        else
            anim_right->Play(player_pos.x, player_pos.y, delta);
    }

    POINT getPlayerPos() const {
        return this->player_pos;
    }

    int getWidth() const {
        return this->PLAY_WIDTH;
    };

    int getHeight() const {
        return this->PLAY_HEIGHT;
    };

    ~Player() {
        delete anim_left;
        delete anim_right;
    }


private:
    IMAGE shadow_player;
    Animation *anim_left;
    Animation *anim_right;

private:
    const int PLAY_WIDTH = 80; // 玩家高度
    const int PLAY_HEIGHT = 80; // 玩家宽度
    const int SHADOW_WIDTH = 32; // 阴影宽度
    POINT player_pos = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
    int player_speed = 5;
    bool is_up = false;
    bool is_down = false;
    bool is_left = false;
    bool is_right = false;
};

class Bullet {
public:
    Bullet() = default;

    ~Bullet() = default;

    POINT pos = {0, 0};

    void Draw() {
        setlinecolor(RGB(255, 155, 50));
        setfillcolor(RGB(200, 75, 10));
        fillcircle(pos.x, pos.y, radius);
    }

private:
    const int radius = 10;
};

class Enemy {
public:
    Enemy() {
        anim_left = new Animation(enemy_left, 45);
        anim_right = new Animation(enemy_right, 45);
        loadimage(&shadow_enemy, TEXT("./assert/img/shadow_enemy.png"));

        enum class SpawnEdge {
            UP = 0,
            DOWN,
            LEFT,
            RIGHT
        };
        SpawnEdge edge = (SpawnEdge) (rand() % 4);

        switch (edge) {
            case SpawnEdge::UP:
                enemy_pos.x = rand() % WINDOW_WIDTH;
                enemy_pos.y = -ENEMY_HEIGHT;
                break;
            case SpawnEdge::DOWN:
                enemy_pos.x = rand() % WINDOW_WIDTH;
                enemy_pos.y = WINDOW_HEIGHT + ENEMY_HEIGHT;
                break;
            case SpawnEdge::LEFT:
                enemy_pos.x = -ENEMY_WIDTH;
                enemy_pos.y = rand() % WINDOW_HEIGHT;
                break;
            case SpawnEdge::RIGHT:
                enemy_pos.x = WINDOW_WIDTH + ENEMY_WIDTH;
                enemy_pos.y = rand() % WINDOW_HEIGHT;
                break;
            default:
                break;
        }
    }

    void Move(const Player &player) {
        POINT p_center = {player.getPlayerPos().x + player.getWidth() / 2,
                          player.getPlayerPos().y + player.getHeight() / 2};
        POINT e_center = {enemy_pos.x + ENEMY_WIDTH / 2,
                          enemy_pos.y + ENEMY_HEIGHT / 2};

        int dir_x = p_center.x - e_center.x;
        facing_left = dir_x < 0;
        int dir_y = p_center.y - e_center.y;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len_dir != 0) {
            double normalized_x = enemy_speed * dir_x / len_dir;
            double normalized_y = enemy_speed * dir_y / len_dir;
            enemy_pos.x += (int) normalized_x;
            enemy_pos.y += (int) normalized_y;
        }
    }

    bool CheckBulletCollision(const Bullet &bullet) {
        if (bullet.pos.x >= enemy_pos.x && bullet.pos.x <= enemy_pos.x + ENEMY_WIDTH &&
            bullet.pos.y >= enemy_pos.y && bullet.pos.y <= enemy_pos.y + ENEMY_HEIGHT)
            return true;
        else
            return false;
    }

    // 只要敌人的中心点在玩家的矩形中即可（就不用矩形碰矩形了）
    bool CheckPlayerCollision(const Player &player) {
        POINT enemy_center = {enemy_pos.x + ENEMY_WIDTH / 2, enemy_pos.y + ENEMY_HEIGHT / 2};
        if (enemy_center.x >= player.getPlayerPos().x &&
            enemy_center.x <= player.getPlayerPos().x + player.getWidth() &&
            enemy_center.y >= player.getPlayerPos().y && enemy_center.y <= player.getPlayerPos().y + player.getHeight())
            return true;
        else
            return false;
    }

    void Draw(int delta) {
        putimage_alpha(enemy_pos.x + ENEMY_WIDTH / 2 - SHADOW_WIDTH / 2,
                       enemy_pos.y + ENEMY_HEIGHT - shadow_enemy.getheight() / 2, shadow_enemy);
        if (facing_left)
            anim_left->Play(enemy_pos.x, enemy_pos.y, delta);
        else
            anim_right->Play(enemy_pos.x, enemy_pos.y, delta);
    }

    void Hurt() {
        alive = false;
    }

    bool CheckAlive() {
        return alive;
    }

    ~Enemy() {
        delete anim_left;
        delete anim_right;
    }


private:
    IMAGE shadow_enemy;
    Animation *anim_left;
    Animation *anim_right;

private:
    const int ENEMY_WIDTH = 80; // 玩家高度
    const int ENEMY_HEIGHT = 80; // 玩家宽度
    const int SHADOW_WIDTH = 32; // 阴影宽度
    POINT enemy_pos = {500, 500};
    int enemy_speed = 2;
    bool facing_left = false;
    bool alive = true;
};

class Button {
private:
    enum class Status {
        idle = 0,
        hovered,
        pushed
    };
    RECT rect;
    IMAGE img_idle;
    IMAGE img_hovered;
    IMAGE img_pushed;
    Status status = Status::idle;

    bool checkMouseHit(int x, int y) {
        return x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
    }

public:
    Button(RECT &rect, std::string path_idle, std::string path_hovered, std::string path_pushed) {
        this->rect = rect;
        loadimage(&img_idle, path_idle.c_str());
        loadimage(&img_hovered, path_hovered.c_str());
        loadimage(&img_pushed, path_pushed.c_str());
    }

    ~Button() = default;

    virtual void Onclick() = 0;

    void processEvent(ExMessage &msg) {
        switch (msg.message) {
            case WM_MOUSEMOVE:
                if (status == Status::idle && checkMouseHit(msg.x, msg.y)) {
                    status = Status::hovered;
                } else if (status == Status::hovered && !checkMouseHit(msg.x, msg.y)) {
                    status = Status::idle;
                }
                break;
            case WM_LBUTTONDOWN:
                if (status == Status::hovered && checkMouseHit(msg.x, msg.y)) {
                    status = Status::pushed;
                }
                break;
            case WM_LBUTTONUP:
                if (status == Status::pushed) {
                    Onclick();
                }
                break;
            default:
                break;
        }
    }

    void Draw() {
        switch (status) {
            case Status::idle:
                putimage(rect.left, rect.top, &img_idle);
                break;
            case Status::hovered:
                putimage(rect.left, rect.top, &img_hovered);
                break;
            case Status::pushed:
                putimage(rect.left, rect.top, &img_pushed);
                break;
        }
    }


};

class StartGameButton : public Button {
public:
    StartGameButton(RECT &rect, std::string path_idle, std::string path_hovered, std::string path_pushed)
            : Button(rect, path_idle, path_hovered, path_pushed) {}

    ~StartGameButton() = default;

protected:
    void Onclick() override {
        is_game_running = true;
        // 播放音乐
        mciSendString(TEXT("play bgm repeat from 0"), NULL, 0, NULL);
    }
};

class QuitGameButton : public Button {
public:
    QuitGameButton(RECT &rect, std::string path_idle, std::string path_hovered, std::string path_pushed)
            : Button(rect, path_idle, path_hovered, path_pushed) {}

    ~QuitGameButton() = default;

protected:
    void Onclick() override {
        isRunning = false;
    }
};

// 生成敌人
void createEnemy(std::vector<Enemy *> &enemys) {
    static int tick = 0;
//        std::cout << "current tick: " << tick << std::endl;
    if (++tick % 144 == 0) {
//            std::cout << "created" << std::endl;
        enemys.push_back(new Enemy());
    }
}

// 更新子弹位置
void updateBullet(std::vector<Bullet> &bullets, const Player &player) {
    const double RADIAL_SPEED = 0.0045; // 径向波动速度
    const double TANGENT_SPEED = 0.0015; // 切向波动速度
    double radian_interval = 2 * 3.1415926 / bullets.size(); // 每个子弹之间的弧度间隔
    POINT player_center = {player.getPlayerPos().x + player.getWidth() / 2,
                           player.getPlayerPos().y + player.getHeight() / 2};
    double radius =
            1.5 * fmax(player.getHeight(), player.getWidth()) + 25 * sin(GetTickCount() * RADIAL_SPEED); // 让半径周期性变化
    for (int i = 0; i < bullets.size(); ++i) {
        double radian = i * radian_interval + GetTickCount() * TANGENT_SPEED; // 弧度也周期性变化
        bullets[i].pos.x = player_center.x + radius * cos(radian);
        bullets[i].pos.y = player_center.y + radius * sin(radian);
    }
}


int main() {
    // 初始化数据
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    player_left = new Atlas("./assert/img/player_left_%d.png", 6);
    player_right = new Atlas("./assert/img/player_right_%d.png", 6);
    enemy_left = new Atlas("./assert/img/enemy_left_%d.png", 6);
    enemy_right = new Atlas("./assert/img/enemy_right_%d.png", 6);

    // loadAnimation();
    IMAGE img; // 背景
    IMAGE start_menu; // 开始菜单背景
    // 加载背景
    loadimage(&img, TEXT("./assert/img/background.png"));
    loadimage(&start_menu, TEXT("./assert/img/menu.png"));

    // 加载音乐
    mciSendString(TEXT("open ./assert/mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(TEXT("open ./assert/mus/hit.wav alias hit"), NULL, 0, NULL);
    Player player;
//    Enemy enemy;
    std::vector<Enemy *> enemys;
    std::vector<Bullet> bullets(3);

    RECT start_game_rect, quit_game_rect;
    start_game_rect.left = (WINDOW_WIDTH - BUTTON_WIDTH)/2;
    start_game_rect.right = start_game_rect.left + BUTTON_WIDTH;
    start_game_rect.top = WINDOW_HEIGHT * 2 / 3 - BUTTON_HEIGHT/2;
    start_game_rect.bottom = start_game_rect.top + BUTTON_HEIGHT;

    quit_game_rect.left = (WINDOW_WIDTH - BUTTON_WIDTH)/2;
    quit_game_rect.right = quit_game_rect.left + BUTTON_WIDTH;
    quit_game_rect.top = start_game_rect.bottom + 20;
    quit_game_rect.bottom = quit_game_rect.top + BUTTON_HEIGHT;


    StartGameButton start_game_button = StartGameButton(start_game_rect, "./assert/img/ui_start_idle.png", "./assert/img/ui_start_hovered.png", "./assert/img/ui_start_pushed.png");
    QuitGameButton quit_game_button = QuitGameButton(quit_game_rect, "./assert/img/ui_quit_idle.png", "./assert/img/ui_quit_hovered.png", "./assert/img/ui_quit_pushed.png");


    // 得分
    int score = 0;
    BeginBatchDraw();

    while (isRunning) {
        // 读取操作
        DWORD beginTime = GetTickCount();
        ExMessage msg;
        while (peekmessage(&msg)) {
            if (is_game_running){
                player.ProcessEvent(msg);
            }else{
                start_game_button.processEvent(msg);
                quit_game_button.processEvent(msg);
            }

        }
        if (is_game_running){
            createEnemy(enemys);
            player.Move();
            updateBullet(bullets, player);
            for (auto enemy = enemys.begin(); enemy != enemys.end(); enemy++) {
                (*enemy)->Move(player);
                // 检测碰撞玩家
                if ((*enemy)->CheckPlayerCollision(player)) {
                    isRunning = false;
                    break;
                }
                // 检测碰撞子弹
                for (Bullet &bullet: bullets) {
                    if ((*enemy)->CheckBulletCollision(bullet)) {
                        (*enemy)->Hurt();
                        mciSendString(TEXT("play hit from 0"), NULL, 0, NULL);
                        break;
                    }
                }
            }
            // 删除死亡的敌人
            for (auto enemy = enemys.begin(); enemy != enemys.end();) {
                if (!((*enemy)->CheckAlive())) {
                    delete *enemy;
                    enemy = enemys.erase(enemy);
                    score++;
                } else {
                    enemy++;
                }
            }
        }


        // 渲染
        cleardevice();
        if (is_game_running){
            putimage(0, 0, &img);
            player.Draw(1000 / 144);
            for (auto enemy: enemys) {
                enemy->Draw(1000 / 144);
            }
            for (Bullet &b: bullets) {
                b.Draw();
            }
        }else{
            putimage(0, 0, &start_menu);
            start_game_button.Draw();
            quit_game_button.Draw();
        }
        FlushBatchDraw();


        DWORD endTime = GetTickCount();
        DWORD divTime = endTime - beginTime;
        if (divTime < 1000 / 144) {
            Sleep(1000 / 144 - divTime);
        }
    }
    std::string result = "游戏结束，得分为:" + std::to_string(score);
    MessageBox(GetHWnd(), result.c_str(), TEXT("哈哈"), MB_OK);

    EndBatchDraw();
    delete player_left;
    delete player_right;
    delete enemy_left;
    delete enemy_right;

    return 0;
}
