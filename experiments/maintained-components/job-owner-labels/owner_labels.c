/* Metadata-only allocation provenance for two actual completed-frame labels.
 * No manager, queue, vector, worker or actor payload is dereferenced. A binding
 * is not a no-free lease: every successful binding retains missing31/session0.
 */
#include <stdint.h>
#include "object_binding.h"

extern volatile uint32_t _gz_gcc_report[280];
extern volatile uint32_t _gz_jwa_exclusive_probe_report[32];
extern volatile uint32_t _gz_gfo_report[32];
extern volatile uint32_t _gz_spo_native_root_slot;
extern const unsigned char _gz_ob_native_text_begin[];

enum { JLP_MAGIC=0x4a4c5031u, JLP_WORDS=128, JLP_ROW=48, JLP_HEAD=32 };
volatile uint32_t _gz_jlp_report[JLP_WORDS]
    __attribute__((section(".data.gz_jlp"))) = {
    JLP_MAGIC,512,1,0,0,0,0,0,0,0,0,0,0,0,2,0,31,0
};

/* Pure format/provenance validation. Caller-supplied labels can exercise this
 * function, but no branch issues a native owner or permission to read them.
 */
uint32_t _gz_jlp_pick(const uint32_t *g, uint32_t serial, uint32_t thread,
                     uint32_t text_base, uint32_t general, uint32_t render,
                     uint32_t *queries)
{
    static const uint32_t callers[4]={0x7f9c70u,0x72b5ecu,0x72b758u,0x7fa320u};
    uint32_t previous=0,job_serial=0,i,j;
    if (!g || !queries || !serial || serial==UINT32_MAX || !thread ||
        (text_base&3u) || !text_base || !general || !render ||
        ((general|render)&3u) || render>UINT32_MAX-608u)
        return 0;
    if (g[0]!=0x47434331u || g[1]!=1120 || g[2]!=1 || (g[3]&1u) ||
        g[4]!=serial || g[5]!=2 || !g[6] || g[7]<g[6] || g[8]!=thread ||
        g[9]!=1 || g[10]!=15 || g[11] || g[12]==UINT32_MAX ||
        g[13]<g[12] || g[13]-g[12]!=4 || g[14]!=4 || g[15]!=4 ||
        g[16]!=4 || g[17] || g[18] || g[19]!=16 || g[20]!=4 ||
        g[21]!=g[22] || g[23])
        return 0;
    for (i=0;i<16;++i) {
        const uint32_t *row=g+24+16*i;
        if (i>=4) {
            for (j=0;j<16;++j) if (row[j]) return 0;
            continue;
        }
        if (callers[i]>UINT32_MAX-text_base || row[0]!=(i==3?2u:1u) ||
            !row[1] || row[2]!=serial || row[3]!=serial ||
            row[4]<g[6] || row[5]<row[4] || row[5]>g[7] ||
            row[6]!=thread || row[7]!=thread ||
            row[9]!=text_base+callers[i] || !row[10] ||
            row[10]<=previous || row[11]<=row[10] ||
            row[12]!=i+1 || row[13] || row[14]==UINT32_MAX ||
            row[15]==UINT32_MAX)
            return 0;
        previous=row[11];
        if (i<3) {
            if (row[1]<=job_serial) return 0;
            job_serial=row[1];
        }
    }
    if (g[24+8]!=render+608u || g[40+8]!=general ||
        g[56+8]!=general || g[72+8]!=render || render+608u==general)
        return 0;
    queries[0]=render+608u;
    queries[1]=general;
    return 1;
}

static void barrier(void) { __asm__ volatile("sync" ::: "memory"); }
static uint32_t binding_format(const volatile uint32_t *b,uint32_t p,uint32_t n)
{
    return b[0]==0x4f424431u && b[1]==64 && b[2]==1 && b[3]==p &&
        b[4]==n && b[5] && b[6]>=n && p>=b[5] &&
        p-b[5]<=b[6]-n && b[7] && b[8] && b[9] &&
        b[13]==31 && !b[14] && !b[15];
}

void _gz_jlp_observe(uint32_t serial,uint32_t thread)
{
    volatile uint32_t *r=_gz_jlp_report;
    uint32_t g[280],queries[2],i,j,seq,gseq,general,render,exit_general,exit_render;
    const uint32_t stride=300u; /* At most one observation per ten game seconds. */
    volatile uint32_t *general_slot=(volatile uint32_t *)
        ((uintptr_t)&_gz_spo_native_root_slot+516u);
    volatile uint32_t *render_slot=(volatile uint32_t *)
        ((uintptr_t)&_gz_spo_native_root_slot+5280u);
    if (!serial || !thread || r[4]==2 || r[31] || r[3]&1u || r[5]==UINT32_MAX ||
        r[3]>UINT32_MAX-3u || (r[9] && serial-r[9]<stride))
        return;
    /* The earlier real TRY/LEAVE must have returned cleanly. This is an OPEN,
     * non-owning metadata observation, never a consumer under that token.
     */
    if (_gz_jwa_exclusive_probe_report[4]!=1 ||
        _gz_jwa_exclusive_probe_report[14]!=serial ||
        _gz_jwa_exclusive_probe_report[15]!=thread ||
        _gz_jwa_exclusive_probe_report[19] ||
        _gz_jwa_exclusive_probe_report[20] ||
        _gz_jwa_exclusive_probe_report[21] ||
        _gz_jwa_exclusive_probe_report[24] ||
        _gz_jwa_exclusive_probe_report[26] ||
        _gz_gfo_report[4]!=serial || _gz_gfo_report[9]!=thread)
        return;
    gseq=_gz_gcc_report[3];
    if (gseq&1u) return;
    for (i=0;i<280;++i) g[i]=_gz_gcc_report[i];
    barrier();
    if (gseq!=g[3] || gseq!=_gz_gcc_report[3] ||
        _gz_gfo_report[7]!=g[7]) return;
    /* Only two fixed, loader-relocated resident RPL DATA words are read. */
    general=*general_slot;render=*render_slot;
    if (!_gz_jlp_pick(g,serial,thread,(uint32_t)(uintptr_t)
                     _gz_ob_native_text_begin,general,render,queries))
        return;
    seq=r[3];
    r[31]=1;r[3]=seq+1;barrier();
    r[4]=1;r[5]++;r[6]=serial;r[7]=g[7];r[8]=thread;r[9]=serial;
    r[10]=gseq;r[11]=general;r[12]=render;
    r[13]=_gz_jwa_exclusive_probe_report[3];r[15]=0;r[16]=31;r[17]=0;
    r[18]=0;r[19]=0;r[20]=0;
    for (i=0;i<2;++i) {
        volatile uint32_t *row=r+JLP_HEAD+JLP_ROW*i;
        uint32_t n=i?128u:112u;
        for (j=0;j<JLP_ROW;++j) row[j]=0;
        row[0]=i+1;row[1]=queries[i];row[2]=n;
        row[3]=row[4]=row[5]=row[6]=UINT32_MAX;
        row[3]=_gz_ob_capture(queries[i],n,(ObBinding *)(row+8));
        if (row[3]==OB_OK) {
            if (!binding_format(row+8,queries[i],n)) {r[4]=2;break;}
            row[4]=_gz_ob_validate((const ObBinding *)(row+8));
            ++r[18];
        }
    }
    exit_general=*general_slot;exit_render=*render_slot;
    r[21]=exit_general;r[22]=exit_render;
    r[20]=exit_general==general && exit_render==render &&
          _gz_gcc_report[3]==gseq && _gz_gcc_report[4]==serial;
    if (r[4]!=2 && r[20]) {
        for (i=0;i<2;++i) {
            volatile uint32_t *row=r+JLP_HEAD+JLP_ROW*i;
            row[5]=_gz_ob_capture(row[1],row[2],(ObBinding *)(row+24));
            if (row[5]==OB_OK) {
                if (!binding_format(row+24,row[1],row[2])) {r[4]=2;break;}
                row[6]=_gz_ob_validate((const ObBinding *)(row+24));
                ++r[19];
            }
            row[7]=row[3]==OB_OK && row[4]==OB_OK &&
                   row[5]==OB_OK && row[6]==OB_OK;
            for (j=0;j<16;++j) if (row[8+j]!=row[24+j]) row[7]=0;
        }
    }
    r[23]=_gz_gcc_report[3];r[24]=*general_slot;r[25]=*render_slot;
    r[20]=r[20] && r[23]==gseq && r[24]==general && r[25]==render;
    /* No query/header/vector dereference and no physical retention followed. */
    r[15]=0;r[16]=31;r[17]=0;r[31]=0;barrier();r[3]=seq+2;
}
