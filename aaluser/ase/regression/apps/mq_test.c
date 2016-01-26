#include "ase_common.h"

int app2sim_rx;
int sim2app_tx;
int app2sim_csr_wr_rx;
int app2sim_umsg_rx;
int app2sim_simkill_rx;

int main()
{
  // Evaluate PWD
  ase_run_path = malloc(ASE_FILEPATH_LEN);
  ase_run_path = getenv("PWD");
#ifdef ASE_DEBUG
  printf("SIM-C : ASE Run path =>\n");
  printf("        %s\n", ase_run_path);
#endif

  // Evaluate Session directory
  ase_workdir_path = malloc(ASE_FILEPATH_LEN);
  /* ase_workdir_path = ase_eval_session_directory();   */
  sprintf(ase_workdir_path, "%s/work/", ase_run_path);
  printf("SIM-C : ASE Session Directory located at =>\n");
  printf("        %s\n", ase_workdir_path);
  printf("SIM-C : ASE Run path =>\n");
  printf("        %s\n", ase_run_path);

  // Evaluate IPCs
  ipc_init();

  // Generate timstamp (used as session ID)
  put_timestamp();
  tstamp_filepath = malloc(ASE_FILEPATH_LEN);
  strcpy(tstamp_filepath, ase_workdir_path);
  strcat(tstamp_filepath, TSTAMP_FILENAME);

  // Print timestamp
  printf("SIM-C : Session ID => %s\n", get_timestamp(0) );

  // Create IPC cleanup setup
  create_ipc_listfile();

  // Set up message queues
  printf("SIM-C : Creating Messaging IPCs...\n");
  int ipc_iter;
  for( ipc_iter = 0; ipc_iter < ASE_MQ_INSTANCES; ipc_iter++)
    mqueue_create( mq_array[ipc_iter].name );
  // ase_mqueue_setup();

  // Open message queues
  app2sim_rx         = mqueue_open(mq_array[0].name,  mq_array[0].perm_flag);
  app2sim_csr_wr_rx  = mqueue_open(mq_array[1].name,  mq_array[1].perm_flag);
  app2sim_umsg_rx    = mqueue_open(mq_array[2].name,  mq_array[2].perm_flag);
  app2sim_simkill_rx = mqueue_open(mq_array[3].name,  mq_array[3].perm_flag);
  sim2app_tx         = mqueue_open(mq_array[4].name,  mq_array[4].perm_flag);

  return 0;
}
