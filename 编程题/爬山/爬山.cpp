#include <iostream>
using namespace std;

enum action {
	init, goUp, goDown, goLeft, goRight
};

// 返回四个结果中的最大值
int maxResult(int upResult, int downResult, int leftResult, int rightResult) {
	int result = 0;

	if (result < upResult)
		result = upResult;

	if (result < downResult)
		result = downResult;

	if (result < leftResult)
		result = leftResult;

	if (result < rightResult)
		result = rightResult;

	return result;
}

// 如果有更高的位置就往过走, 无处可去则返回自身所在位置的高度
//  arr: 二维数组; n, m: 二维数组行列; i, j: 当前位置; lastAction: 上一步的行动
int doClimb(int** arr, int n, int m, int i, int j, int lastAction) {
	                                                                // 如果
	if ((i == 0 || arr[i - 1][j] < arr[i][j]) &&					// 不能往上走
		(i == n - 1 || arr[i + 1][j] < arr[i][j]) &&				// 不能往下走
		(j == 0 || arr[i][j - 1] < arr[i][j]) &&					// 不能往左走
		(j == m - 1 || arr[i][j + 1] < arr[i][j])) {				// 不能往右走
		return arr[i][j];										    // 返回当前位置的高度
	}

	int upResult = 0, downResult = 0, leftResult = 0, rightResult = 0;

	if (!(i == 0 || arr[i - 1][j] < arr[i][j])) {				    // 能往上走
		if (lastAction != goDown) {									// 上一步不是往下走(防止循环)
			upResult = doClimb(arr, n, m, i - 1, j, goUp);			// 往上走
		}														    // 下同
	}

	if (!(i == n - 1 || arr[i + 1][j] < arr[i][j])) {
		if (lastAction != goUp) {
			downResult = doClimb(arr, n, m, i + 1, j, goDown);		// 往下走
		}
	}

	if (!(j == 0 || arr[i][j - 1] < arr[i][j])) {
		if (lastAction != goRight) {
			leftResult = doClimb(arr, n, m, i, j - 1, goLeft);		// 往左走
		}
	}

	if (!(j == m - 1 || arr[i][j + 1] < arr[i][j])) {
		if (lastAction != goLeft) {
			rightResult = doClimb(arr, n, m, i, j + 1, goRight);	// 往右走
		}
	}

	return maxResult(upResult, downResult, leftResult, rightResult);
}

int main() {
	int n = 0, m = 0;
	int posi = 0, posj = 0;
	int i = 0, j = 0;

	cin >> n >> m;					// n行m列
	cin >> posi >> posj;			// 初始位置

	int** arr = new int*[n];	    // 申请n行m列的二维数组
	for (i = 0; i<n; i++) {
		arr[i] = new int[m];

		for (j = 0; j<m; j++)		// 输入对应的高度
			cin >> arr[i][j];
	}

	cout << doClimb(arr, n, m, posi-1, posj-1, init);	// 题目中第一行列是1, 但数组下标是0

	for (i = 0; i<n; i++)
		delete[] arr[i];
	delete[] arr;

	// system("pause");
	return 0;
}