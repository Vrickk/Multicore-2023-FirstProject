import pygame
import random

# 미로 설정
width, height = 25, 25  # 미로 크기
cell_size = 20         # 셀 크기
maze = []
for i in range(height):
    row = []
    for j in range(width):
        row.append('#')
    maze.append(row)
    
# 첫 행과 마지막 행, 첫 열과 마지막 열은 벽
for i in range(width):
    maze[0][i] = '#'
    maze[height-1][i] = '#'
for i in range(height):
    maze[i][0] = '#'
    maze[i][width-1] = '#'

# 시작점과 끝점은 벽이 아님
maze[0][width - 2] = 'S'
maze[height - 1][1] = 'E'
    
# 미로 생성
stack = [(1, 1)]
while len(stack) > 0:
    (y, x) = stack[-1]
    neighbors = []
    if x > 2 and maze[y][x - 2] == '#':
        neighbors.append((y, x - 2))
    if y > 2 and maze[y - 2][x] == '#':
        neighbors.append((y - 2, x))
    if x < width - 3 and maze[y][x + 2] == '#':
        neighbors.append((y, x + 2))
    if y < height - 3 and maze[y + 2][x] == '#':
        neighbors.append((y + 2, x))
    if len(neighbors) == 0:
        stack.pop()
    else:
        (ny, nx) = random.choice(neighbors)
        if ny == y:
            maze[y][(nx + x) // 2] = ' '
        else:
            maze[(ny + y) // 2][x] = ' '
        maze[ny][nx] = ' '
        stack.append((ny, nx))

with open('maze.txt', 'w') as f:
    for row in maze:
        f.write(''.join(row) + '\n')

# 미로 시각화
pygame.init()
screen = pygame.display.set_mode((width * cell_size, height * cell_size))
clock = pygame.time.Clock()
while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            quit()

    screen.fill((255, 255, 255))
    for i in range(height):
        for j in range(width):
            if maze[i][j] == '#':
                pygame.draw.rect(screen, (0, 0, 0), (j * cell_size, i * cell_size, cell_size, cell_size))
            elif maze[i][j] == 'S':
                pygame.draw.rect(screen, (255, 0, 0), (j * cell_size, i * cell_size, cell_size, cell_size))
            elif maze[i][j] == 'E':
                pygame.draw.rect(screen, (0, 255, 0), (j * cell_size, i * cell_size, cell_size, cell_size))
            else:
                pygame.draw.rect(screen, (255, 255, 255), (j * cell_size, i * cell_size, cell_size, cell_size))
    pygame.display.update()
    clock.tick(60)