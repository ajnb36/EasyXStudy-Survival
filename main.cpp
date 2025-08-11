#include <cmath>
#include <graphics.h>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>


// int idx_current_anim = 0;
// const int PLAY_ANIM_NUM = 6; // 动画帧总数为6
// IMAGE play_left[PLAY_ANIM_NUM];
// IMAGE play_right[PLAY_ANIM_NUM];

// const int PLAY_WIDTH = 80; // 玩家高度
// const int PLAY_HEIGHT = 80; // 玩家宽度
// const int SHADOW_WIDTH = 32; // 阴影宽度
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
// POINT player_pos = {500, 500};
// int player_speed = 5;

// bool is_up = false;
// bool is_down = false;
// bool is_left = false;
// bool is_right = false;
// IMAGE *what_direct;

void putimage_alpha(int x, int y, IMAGE &img) {
    int w = img.getwidth();
    int h = img.getheight();
    AlphaBlend(GetImageHDC(NULL), x, y, w, h,
               GetImageHDC(&img), 0, 0, w, h, {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA});
}


class Animation {
public:
    Animation(std::string path, int num, int interval) : interval_ms(interval) {
        TCHAR file_path[MAX_PATH];
        for (int i = 0; i < num; i++) {
            _stprintf_s(file_path, path.c_str(), i);
            IMAGE *img = new IMAGE();
            loadimage(img, file_path);
            frame_list.push_back(img);
        }
    }

    void Play(int x, int y, int delta) {
        timer += delta;
        if (timer >= interval_ms) {
            idx_frame = (idx_frame + 1) % (int) frame_list.size();
            timer = 0;
        }
        putimage_alpha(x, y, *frame_list[idx_frame]);
    }

    ~Animation() {
        for (int i = 0; i < frame_list.size(); i++) {
            delete frame_list[i];
        }
    }

private:
    int timer = 0; // 动画计时器
    int interval_ms = 0; // 间隔时间 单位ms
    int idx_frame = 0; // 动画帧索引
    std::vector<IMAGE *> frame_list;
};

class Player {
public:
    Player() {
        anim_left = new Animation("./assert/img/player_left_%d.png", 6, 45);
        anim_right = new Animation("./assert/img/player_right_%d.png", 6, 45);
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
    POINT player_pos = {500, 500};
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
        anim_left = new Animation("./assert/img/enemy_left_%d.png", 6, 45);
        anim_right = new Animation("./assert/img/enemy_right_%d.png", 6, 45);
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




// Animation play_left("./assert/img/player_left_%d.png", 6, 45);
// Animation play_right("./assert/img/player_right_%d.png", 6, 45);
// IMAGE shadow_player;

// void draw_player(int delta, int dir_x) {
//     putimage_alpha(player_pos.x + PLAY_WIDTH / 2 - SHADOW_WIDTH / 2,
//                    player_pos.y + PLAY_HEIGHT - shadow_player.getheight() / 2, shadow_player);
//
//     static bool facing_left = false;
//     if (dir_x > 0)
//         facing_left = false;
//     else if (dir_x < 0)
//         facing_left = true;
//     if (facing_left)
//         play_left.Play(player_pos.x, player_pos.y, delta);
//     else
//         play_right.Play(player_pos.x, player_pos.y, delta);
// }

// void loadAnimation() {
//     for (int i = 0; i < PLAY_ANIM_NUM; i++) {
//         std::string str = TEXT("./assert/img/player_left_"+std::to_string(i)+".png");
//         loadimage(&play_left[i], str.c_str());
//         str = TEXT("./assert/img/player_right_"+std::to_string(i)+".png");
//         loadimage(&play_right[i], str.c_str());
//     }
// }

// 生成敌人
void createEnemy(std::vector<Enemy *> &enemys) {
    static int tick = 0;
//        std::cout << "current tick: " << tick << std::endl;
    if (++tick % 144 == 0) {
//            std::cout << "created" << std::endl;
        enemys.push_back(new Enemy());
    }
}

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
    // loadAnimation();
    IMAGE img;
    // 加载背景
    loadimage(&img, TEXT("./assert/img/background.png"));
    // what_direct = play_left;
    // loadimage(&shadow_player, TEXT("./assert/img/shadow_player.png"));
    Player player;
//    Enemy enemy;
    std::vector<Enemy *> enemys;
    std::vector<Bullet> bullets(3);
    // 得分
    int score = 0;
    BeginBatchDraw();

    bool isRunning = true;
    while (isRunning) {
        // 读取操作
        DWORD beginTime = GetTickCount();
        ExMessage msg;
        while (peekmessage(&msg)) {
//            if (msg.message == WM_KEYDOWN) {
//                switch (msg.vkcode) {
//                    case VK_LEFT:
//                        is_left = true;
//                        break;
//                    case VK_RIGHT:
//                        is_right = true;
//                        break;
//                    case VK_UP:
//                        is_up = true;
//                        break;
//                    case VK_DOWN:
//                        is_down = true;
//                        break;
//                    default: ;
//                }
//            } else if (msg.message == WM_KEYUP) {
//                switch (msg.vkcode) {
//                    case VK_LEFT:
//                        is_left = false;
//                        break;
//                    case VK_RIGHT:
//                        is_right = false;
//                        break;
//                    case VK_UP:
//                        is_up = false;
//                        break;
//                    case VK_DOWN:
//                        is_down = false;
//                        break;
//                    default: ;
//                }
//            }
            player.ProcessEvent(msg);
        }

        // 处理数据
        // 为保证斜着走时，也能保证速度和单方向走时一致，需要对速度进行归一化处理
//        int dir_x = is_right - is_left;
//        int dir_y = is_down - is_up;
//        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
//        if (len_dir != 0) {
//            double normalized_x = player_speed * dir_x / len_dir;
//            double normalized_y = player_speed * dir_y / len_dir;
//            player_pos.x += normalized_x;
//            player_pos.y += normalized_y;
//        }
//
//        // 保证玩家不出界
//        if (player_pos.x < 0) player_pos.x = 0;
//        if (player_pos.y < 0) player_pos.y = 0;
//        if (player_pos.x + PLAY_WIDTH > WINDOW_WIDTH) player_pos.x = WINDOW_WIDTH - PLAY_WIDTH;
//        if (player_pos.y + PLAY_HEIGHT > WINDOW_HEIGHT) player_pos.y = WINDOW_HEIGHT - PLAY_HEIGHT;
        // 生成敌人
//        static int tick = 0;
//        tick++;
////        std::cout << "current tick: " << tick << std::endl;
//        if (tick % 144 == 0){
////            std::cout << "created" << std::endl;
//            enemys.push_back(new Enemy());
//        }
        createEnemy(enemys);

        player.Move();
//        enemy.Move(player);
        updateBullet(bullets, player);
        for (auto enemy = enemys.begin(); enemy != enemys.end();enemy++) {
//            bool is_delete = false;
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
//                    delete *enemy;
//                    enemy = enemys.erase(enemy);
//                    is_delete = true;
                    break;
                }
            }
//            if (is_delete){
//                is_delete = false;
//            }else{
//                enemy++;
//            }
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



        // if (is_left) player_pos.x -= player_speed;
        // if (is_right) player_pos.x += player_speed;
        // if (is_up) player_pos.y -= player_speed;
        // if (is_down) player_pos.y += player_speed;


        // static int count = 0;
        // if (++count % 5 == 0)
        //     idx_current_anim++;
        // // 使动画循环播放
        // // idx_current_anim = idx_current_anim % PLAY_ANIM_NUM;

        cleardevice();
        // 渲染
        putimage(0, 0, &img);
//        draw_player(1000 / 144, is_right - is_left);
        // if (is_left) what_direct = play_left;
        // if (is_right) what_direct = play_right;
        // putimage_alpha(player_pos.x, player_pos.y, what_direct[idx_current_anim]);
        player.Draw(1000 / 144);
//        enemy.Draw(1000 / 144);
        for (auto enemy: enemys) {
            enemy->Draw(1000 / 144);
        }
        for (Bullet &b: bullets) {
            b.Draw();
        }
        FlushBatchDraw();
        DWORD endTime = GetTickCount();
        DWORD divTime = endTime - beginTime;
        if (divTime < 1000 / 144) {
            Sleep(1000 / 144 - divTime);
        }
    }
    std::string result = "游戏结束，得分为:"+std::to_string(score);
    MessageBox(GetHWnd(), result.c_str(), TEXT("哈哈"), MB_OK);

    EndBatchDraw();
    return 0;
}
