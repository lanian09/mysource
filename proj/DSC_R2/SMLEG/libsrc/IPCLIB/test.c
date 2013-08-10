int p( int semid )
{
    struct sembuf p_buf;

    p_buf.sem_num = 0;
    p_buf.sem_op  = -1;
    p_buf.sem_flg = SEM_UNDO;

    if( semop(semid, &p_buf, 1) == -1 )
        return -1;
    else
        return 0;
}

int v( int semid )
{
    struct sembuf v_buf;

    v_buf.sem_num = 0;
    v_buf.sem_op  = 1;
    v_buf.sem_flg = SEM_UNDO;

    if( semop(semid, &v_buf, 1) == -1 )
        return -1;
    else
        return 0;

}
