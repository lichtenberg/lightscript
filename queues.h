/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Handy doubly-linked queue                File: lightscript.h
    *  
    *  We keep lists of stuff everywhere.  this makes it easy.
    *  
    *  Author:  Mitch Lichtenberg
    ********************************************************************* */



typedef struct dqueue_s {
    struct dqueue_s *dq_next;
    struct dqueue_s *dq_prev;
} dqueue_t;


static inline void dq_init(dqueue_t *q) 
{
   q->dq_next = q; 
   q->dq_prev = q;
}


static inline void dq_enqueue(dqueue_t *qb,dqueue_t *item)
{
    qb->dq_prev->dq_next = item;
    item->dq_next = qb;
    item->dq_prev = qb->dq_prev;
    qb->dq_prev = item;
}


static inline void dq_dequeue(dqueue_t *item)
{
    item->dq_prev->dq_next = item->dq_next;
    item->dq_next->dq_prev = item->dq_prev;
}


