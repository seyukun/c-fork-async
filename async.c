#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

void	ft_putendl_fd(char *s, int fd)
{
	if (!s)
		return ;
	write(fd, s, strlen(s));
	write(fd, "\n", 1);
}

void	ft_putnbr_fd(int n, int fd)
{
	unsigned int	unb;
	const char		number[] = "0123456789";

	unb = (unsigned int)n;
	if (n < 0)
	{
		write(fd, "-", 1);
		unb = unb * -1;
	}
	if (unb / 10 > 0)
		ft_putnbr_fd(unb / 10, fd);
	write(fd, &number[unb % 10], 1);
}

void	ft_putstr_fd(char *s, int fd)
{
	if (!s)
		return ;
	write(fd, s, strlen(s));
}

int	error_print(const char *name)
{
	ft_putstr_fd(": minishell: ", 2);
	ft_putstr_fd((char *)name, 2);
	if (errno == EACCES)
		ft_putendl_fd(":Permission denied.", 2);
	else if (errno == EEXIST)
		ft_putendl_fd(":File exists.", 2);
	else if (errno == EINVAL)
		ft_putendl_fd(":Invalid argument.", 2);
	else if (errno == ENOENT)
		ft_putendl_fd(":No such file or directory.", 2);
	else if (errno == ENOMEM)
		ft_putendl_fd(":Not enough space/cannot allocate memory.", 2);
	else if (errno == EBADF)
		ft_putendl_fd(":fd isn't a valid open file descriptor.", 2);
	else if (errno == EIO)
		ft_putendl_fd(":Remote I/O error.", 2);
	else if (errno == E2BIG)
		ft_putendl_fd(":Argument list too long.", 2);
	else
	{
		ft_putstr_fd(":(", 2);
		ft_putnbr_fd(errno, 2);
		ft_putendl_fd(")Unsensitive error.", 2);
	}
	return (1);
}

int	error_print_int(int num)
{
	char	str[2];

	str[1] = '\0';
	str[0] = '0' + num;
	return (error_print(str));
}

void	close_pipe(int *pipe)
{
	if (close(pipe[0]) == -1)
		error_print_int(pipe[0]);
	if (close(pipe[1]) == -1)
		error_print_int(pipe[1]);
}

pid_t	async(char *const *argv, char *const *envp, int is_pipe)
{
	pid_t	pid;
	int		rpipe[2];

	if (argv == NULL || argv[0] == NULL)
		return (0);
	if (is_pipe)
		pipe(rpipe);
	pid = fork();
	if (pid == 0)
	{
		if (is_pipe)
		{
			if (dup2(rpipe[1], STDOUT_FILENO) != -1)
				close_pipe(rpipe);
			else
			{
				error_print("dup2");
				exit(1);
			}
		}
		execve(argv[0], argv, envp);
		error_print(argv[0]);
		exit(1);
	}
	if (is_pipe)
	{
		if (dup2(rpipe[0], STDIN_FILENO) != -1)
			close_pipe(rpipe);
		else
			error_print("dup2");
	}
	return (pid);
}

int	await(pid_t pid)
{
	int	stat;

	if (waitpid(pid, &stat, 0) == pid)
	{
		if (WIFEXITED(stat))
			stat = WEXITSTATUS(stat);
		else if (WIFSIGNALED(stat))
			stat = WTERMSIG(stat);
		return (stat);
	}
	return (1);
}

int	promise(pid_t *pids)
{
	int	stats;

	stats = 1;
	while (pids && *pids)
		stats = await(*pids++);
	return (stats);
}

int	main(void)
{
	pid_t		pids[4];
	const int	infd = dup(STDIN_FILENO);
	char *const	envp[] = {NULL};
	char *const	cmd1[] = {"/bin/ls", "-la", NULL};
	char *const	cmd2[] = {"/bin/grep", ".md", NULL};
	char *const	cmd3[] = {"/bin/cat", NULL};

	bzero(pids, sizeof(pid_t) * 4);
	pids[0] = async(cmd1, envp, 1);
	pids[1] = async(cmd2, envp, 1);
	pids[2] = async(cmd3, envp, 0);
	dup2(infd, STDIN_FILENO);
	promise(pids);
	return (0);
}
