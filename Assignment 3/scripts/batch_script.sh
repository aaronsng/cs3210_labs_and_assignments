#!/bin/bash

for ((i=1; i<=4; i++)); do
    for ((task_id=1; task_id<=3; task_id++)); do
        # ========================================= Variables Initialization ===================================
        # NOTE: Update "num_mpi_processes" to "map_reduce_task_id" variables accordingly to suit your need.
        # Ensure that there is no space before and after "=".
        # Ensure that there is no space in the executable name.
        num_mpi_processes=$(($i + 2 * $i + 1))
        executable="a03"
        num_input_files=9
        num_map_workers=$i
        num_reduce_workers=$((2 * $i))
        map_reduce_task_id=$task_id

        # You don't have to update the "username" variable. You shouldn't have to update the variables here.
        username="$USER"
        local_input_files_dir="./sample_input_files"
        nfs_dir="/nfs/home/${username}/A03"
        remote_input_files_dir="${nfs_dir}/testcases"
        stdouterr_file="stdouterr_${SLURM_JOB_ID}_${i}_${task_id}.txt"
        output_file="output_${SLURM_JOB_ID}_${i}_${task_id}.out"

        nodes=($(scontrol show hostname ${SLURM_NODELIST}))

        # ============================================== Do not modify this part ===========================================
        # Copy output files to the nfs directory
        cleanup() {
	        mv "$stdouterr_file" "$output_file" "$nfs_dir"
        }

        # ============================================== Checking ===========================================
        # Check if executable is in NFS dir
        if ! [[ -e "${nfs_dir}/${executable}" ]]
        then
	        echo "${executable} does not exist in ${nfs_dir}!" >> "$stdouterr_file"
	        cleanup
	        exit 1
        fi

        # Check if sample input files is in NFS dir
        if ! [[ -e "$remote_input_files_dir" ]]
        then
	        echo "${remote_input_files_dir} does not exist" >> "$stdouterr_file"
	        cleanup
	        exit 1
        fi


        # ============================================== Preparation ===========================================
        # Broadcast executable to all allocated nodes
        cd ~
        sbcast -f "${nfs_dir}/${executable}" "/home/${username}/${executable}"

        # Clean up previous input files if any
        [[ -d "$local_input_files_dir" ]] && rm -r "$local_input_files_dir"

        # Copy input files from nfs to local directory
        cp -r "$remote_input_files_dir" "$local_input_files_dir"

        # Print the list of allocated nodes to the stdouterr file
        echo "List of allocated nodes" >> "$stdouterr_file"
        echo "${nodes[@]}" >> "$stdouterr_file"


        # ============================================== Execution =============================================
        # Run the mpi executable and redirect stdout and stderr to another file.
        # The -mca btl... option is required to deal with some communication issues of node 22 to 24.
        mpirun -np "$num_mpi_processes" -mca btl_tcp_if_exclude docker0,docker_gwbridge,lo "$executable" "$local_input_files_dir" "$num_input_files" "$num_map_workers" "$num_reduce_workers" "$output_file" "$map_reduce_task_id" &>> "$stdouterr_file"

        mpi_exit_code=$?

        # ============================================== Correctness ===========================================
        # Correct correctness of the task
        echo "Checking correctness of the program" >> "$stdouterr_file"
        sort "${output_file}" | diff "correct_outputs/${task_id}.out" >> "$stdouterr_file"
        echo "Program's correctness has been checked." >> "$stdouterr_file"

        # ============================================== Clean Up ==============================================
        # Copy output files to the nfs directory
        cleanup

        if [[ "$mpi_exit_code" -ne 0 ]]
        then
	        echo "MPI job fails. Please refer to the output file in your nfs home directory for details of why the job fails." >> "$stdouterr_file"
	        exit 1
        fi
    done
done
