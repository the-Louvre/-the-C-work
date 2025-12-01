#include <stdio.h>
#include <unistd.h>     // usleep
#include "sequencer.h"

// 回调函数：当前有音符要播放时调用
void print_note_callback(int track_id, int channel, const Note *note) {
    printf("[Tick %3d] Track %d (ch %d): pitch=%d, dur=%d, vel=%d\n",
           note->tick, track_id, channel,
           note->pitch, note->duration, note->velocity);
}

// 找到指定 track_id 的轨道
Track* find_track_by_id(Sequencer *seq, int track_id) {
    if (!seq) return NULL;
    Track *t = seq->tracks_head;
    while (t) {
        if (t->track_id == track_id) return t;
        t = t->next;
    }
    return NULL;
}

// 列出所有轨道
void list_tracks(Sequencer *seq, Track *current_track) {
    printf("\n=== 当前轨道列表 ===\n");
    if (!seq->tracks_head) {
        printf("(暂无轨道)\n");
        return;
    }
    Track *t = seq->tracks_head;
    while (t) {
        printf("  [%d] %s (ch %d)%s\n",
               t->track_id, t->name, t->channel,
               (t == current_track ? "  <-- 当前选择" : ""));
        t = t->next;
    }
}

// 列出某个轨道上的音符
void list_notes_of_track(Track *track) {
    if (!track) {
        printf("当前未选择轨道。\n");
        return;
    }
    printf("\n=== 轨道 %d (%s) 的音符 ===\n", track->track_id, track->name);
    if (!track->notes_head) {
        printf("(暂无音符)\n");
        return;
    }
    Note *n = track->notes_head;
    while (n) {
        printf("  tick=%d, pitch=%d, dur=%d, vel=%d\n",
               n->tick, n->pitch, n->duration, n->velocity);
        n = n->next;
    }
}

// 播放整个序列（从 tick 0 到 total_ticks）
void play_sequence(Sequencer *seq) {
    if (!seq) return;
    printf("\n>>> 开始播放...\n");
    seq_reset_play_cursor(seq);

    double ms_per_tick = 60000.0 / (seq->bpm * seq->ticks_per_beat);

    for (int tick = 0; tick < seq->total_ticks; ++tick) {
        seq_step_play(seq, tick, print_note_callback);
        usleep((useconds_t)(ms_per_tick * 1000)); // ms -> us
    }
    printf(">>> 播放结束。\n\n");
}

// 清理输入缓冲区
void flush_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void print_menu(void) {
    printf("========== Cyber Sequencer ==========\n");
    printf("1. 列出所有轨道\n");
    printf("2. 选择当前轨道\n");
    printf("3. 查看当前轨道的音符\n");
    printf("4. 给当前轨道添加音符\n");
    printf("5. 删除当前轨道某个音符\n");
    printf("6. 播放整个序列\n");
    printf("7. 退出\n");
    printf("=====================================\n");
    printf("请输入操作编号: ");
}

int main(void) {
    Sequencer seq;
    seq_init(&seq, 120, 4, 32);   // 120bpm, 4tick/beat, 32 tick 总长度
    Track *current_track = NULL;

    // 预置两个轨道，方便演示
    Track *drums = seq_add_track(&seq, "Drums", 1);
    Track *bass  = seq_add_track(&seq, "Bass",  2);
    current_track = drums;

    int running = 1;

    while (running) {
        print_menu();

        int choice = 0;
        if (scanf("%d", &choice) != 1) {
            printf("输入无效，请重新输入。\n");
            flush_line();
            continue;
        }
        flush_line(); // 清掉行尾的换行

        switch (choice) {
            case 1:  // 列出轨道
                list_tracks(&seq, current_track);
                break;

            case 2: { // 选择轨道
                list_tracks(&seq, current_track);
                printf("请输入要选择的轨道 ID: ");
                int id;
                if (scanf("%d", &id) != 1) {
                    printf("输入无效。\n");
                    flush_line();
                    break;
                }
                flush_line();
                Track *t = find_track_by_id(&seq, id);
                if (t) {
                    current_track = t;
                    printf("已选择轨道 %d (%s)\n", t->track_id, t->name);
                } else {
                    printf("未找到该轨道。\n");
                }
                break;
            }

            case 3:  // 查看当前轨道音符
                list_notes_of_track(current_track);
                break;

            case 4: { // 添加音符
                if (!current_track) {
                    printf("当前未选择任何轨道，无法添加音符。\n");
                    break;
                }
                int tick, pitch, dur, vel;
                printf("请输入 tick pitch duration velocity (以空格分隔): ");
                if (scanf("%d %d %d %d", &tick, &pitch, &dur, &vel) != 4) {
                    printf("输入无效。\n");
                    flush_line();
                    break;
                }
                flush_line();
                if (seq_add_note(current_track, tick, pitch, dur, vel)) {
                    printf("已在轨道 %d (%s) 上添加音符。\n",
                           current_track->track_id, current_track->name);
                } else {
                    printf("添加音符失败。\n");
                }
                break;
            }

            case 5: { // 删除音符
                if (!current_track) {
                    printf("当前未选择任何轨道，无法删除音符。\n");
                    break;
                }
                int tick, pitch;
                printf("请输入要删除音符的 tick 和 pitch: ");
                if (scanf("%d %d", &tick, &pitch) != 2) {
                    printf("输入无效。\n");
                    flush_line();
                    break;
                }
                flush_line();
                if (seq_remove_note_at_tick(current_track, tick, pitch)) {
                    printf("已删除该音符。\n");
                } else {
                    printf("未找到对应的音符。\n");
                }
                break;
            }

            case 6:  // 播放
                play_sequence(&seq);
                break;

            case 7:  // 退出
                running = 0;
                break;

            default:
                printf("未知选项，请重新输入。\n");
                break;
        }
    }

    seq_free(&seq);
    printf("已退出程序。\n");
    return 0;
}