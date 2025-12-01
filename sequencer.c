#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sequencer.h"
void seq_init(Sequencer *seq, int bpm, int ticks_per_beat, int total_ticks) {
    if (!seq) return;
    seq->tracks_head = NULL;
    seq->bpm = bpm;
    seq->ticks_per_beat = ticks_per_beat;
    seq->total_ticks = total_ticks;
}

static void free_notes(Note *head) {
    while (head) {
        Note *tmp = head;
        head = head->next;
        free(tmp);
    }
}

void seq_free(Sequencer *seq) {
    if (!seq) return;
    Track *t = seq->tracks_head;
    while (t) {
        Track *tmp = t;
        free_notes(t->notes_head);
        t = t->next;
        free(tmp);
    }
    seq->tracks_head = NULL;
}
Track* seq_add_track(Sequencer *seq, const char *name, int channel) {
    if (!seq) return NULL;

    Track *t = (Track *)malloc(sizeof(Track));
    if (!t) {
        fprintf(stderr, "Failed to allocate Track\n");
        return NULL;
    }

    static int next_track_id = 1;  // 简单自增 ID
    t->track_id = next_track_id++;
    strncpy(t->name, name ? name : "Track", sizeof(t->name) - 1);
    t->name[sizeof(t->name) - 1] = '\0';
    t->channel = channel;
    t->notes_head = NULL;
    t->play_cursor = NULL;
    t->next = NULL;

    if (!seq->tracks_head) {
        seq->tracks_head = t;
    } else {
        Track *cur = seq->tracks_head;
        while (cur->next) cur = cur->next;
        cur->next = t;
    }

    return t;
}
int seq_remove_track(Sequencer *seq, int track_id) {
    if (!seq || !seq->tracks_head) return 0;

    Track *prev = NULL;
    Track *cur = seq->tracks_head;

    while (cur) {
        if (cur->track_id == track_id) {
            if (prev) prev->next = cur->next;
            else seq->tracks_head = cur->next;

            free_notes(cur->notes_head);
            free(cur);
            return 1; // 成功
        }
        prev = cur;
        cur = cur->next;
    }
    return 0; // 没找到
}
int seq_add_note(Track *track, int tick, int pitch, int duration, int velocity) {
    if (!track) return 0;
    if (tick < 0 || duration <= 0) return 0;

    Note *n = (Note *)malloc(sizeof(Note));
    if (!n) {
        fprintf(stderr, "Failed to allocate Note\n");
        return 0;
    }

    n->tick = tick;
    n->pitch = pitch;
    n->duration = duration;
    n->velocity = velocity;
    n->next = NULL;

    // 有序插入（按 tick 从小到大），tick 相同可以放在后面
    Note *prev = NULL;
    Note *cur = track->notes_head;

    while (cur && cur->tick <= tick) {
        prev = cur;
        cur = cur->next;
    }

    if (!prev) {
        // 插到链表头
        n->next = track->notes_head;
        track->notes_head = n;
    } else {
        prev->next = n;
        n->next = cur;
    }

    return 1;
}
int seq_remove_note_at_tick(Track *track, int tick, int pitch) {
    if (!track || !track->notes_head) return 0;

    Note *prev = NULL;
    Note *cur = track->notes_head;
    while (cur) {
        if (cur->tick == tick && cur->pitch == pitch) {
            if (prev) prev->next = cur->next;
            else track->notes_head = cur->next;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}
void seq_step_play(Sequencer *seq, int current_tick, note_callback cb) {
    if (!seq || !cb) return;

    Track *t = seq->tracks_head;
    while (t) {
        // 如果还没设置播放游标，从头开始
        if (!t->play_cursor)
            t->play_cursor = t->notes_head;

        // 游标前移到 >= current_tick 的位置
        while (t->play_cursor && t->play_cursor->tick < current_tick) {
            t->play_cursor = t->play_cursor->next;
        }

        // 如果当前 tick 有音符，就触发回调
        Note *p = t->play_cursor;
        while (p && p->tick == current_tick) {
            cb(t->track_id, t->channel, p);
            p = p->next;
        }

        t = t->next;
    }
}
void seq_reset_play_cursor(Sequencer *seq) {
    if (!seq) return;
    Track *t = seq->tracks_head;
    while (t) {
        t->play_cursor = NULL;
        t = t->next;
    }
}