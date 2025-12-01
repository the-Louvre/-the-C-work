#ifndef SEQUENCER_H
#define SEQUENCER_H

// ---------- 数据结构定义 ----------

typedef struct Note {
    int tick;          // 所在节拍位置（第几个 tick）
    int pitch;         // 音高（先用 int，后面你想用 MIDI 也行）
    int duration;      // 持续的 tick 数
    int velocity;      // 力度（0-127）
    struct Note *next;
} Note;

typedef struct Track {
    int track_id;          // 轨道编号
    char name[32];         // 乐器名
    int channel;           // 通道（给前端/音频模块用）

    Note *notes_head;      // 子链表头
    Note *play_cursor;     // 播放时用的游标
    struct Track *next;
} Track;

typedef struct Sequencer {
    Track *tracks_head;    // 主链表头
    int bpm;               // 速度（beats per minute）
    int ticks_per_beat;    // 每拍多少 tick
    int total_ticks;       // 总 tick 数
} Sequencer;

// ---------- 对外 API 声明 ----------

// 初始化 / 销毁
void seq_init(Sequencer *seq, int bpm, int ticks_per_beat, int total_ticks);
void seq_free(Sequencer *seq);

// 轨道操作
Track* seq_add_track(Sequencer *seq, const char *name, int channel);
int    seq_remove_track(Sequencer *seq, int track_id);

// 音符操作（按 tick 有序插入）
int seq_add_note(Track *track, int tick, int pitch, int duration, int velocity);
int seq_remove_note_at_tick(Track *track, int tick, int pitch);
// 重置所有轨道的 play_cursor，在每次播放前调用
void seq_reset_play_cursor(Sequencer *seq);
// 播放回调类型
typedef void (*note_callback)(int track_id, int channel, const Note *note);

// 在某个 tick 上触发所有要播放的音符
void seq_step_play(Sequencer *seq, int current_tick, note_callback cb);

#endif // SEQUENCER_H