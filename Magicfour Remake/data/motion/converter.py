import sys

# 파일 경로 설정
if len(sys.argv) != 2:
    print("Usage: python HELP.PY <input_file>")
    sys.exit(1)

input_file = sys.argv[1]  # 입력 파일 경로
output_file = "out.bvh"  # 출력 파일 경로

offset_on = False
offset = None

# 파일 읽기 및 처리
with open(input_file, "r") as infile, open(output_file, "w") as outfile:
    for line in infile:
        if offset_on == True:
            # 각 줄을 공백으로 나누어 리스트로 변환
            numbers = line.split()
            if offset is None and numbers:
                offset = [0,0,0]
                offset[0] = float(numbers[0])
                offset[1] = float(numbers[1])
                offset[2] = float(numbers[2])
            if numbers:  # 빈 줄이 아닌 경우
                numbers[0] = str((float(numbers[0]) - offset[0]) / 40)
                numbers[1] = str((float(numbers[1]) - offset[1]) / 40 + 1.9) 
                numbers[2] = str((float(numbers[2]) - offset[2]) / 40)
            # 처리된 줄을 출력 파일에 작성
            outfile.write(" ".join(numbers) + "\n")
        else:
            outfile.write(line)
            
        if line.startswith("Frame Time:"):
            offset_on = True