#define _CRT_SECURE_NO_WARNINGS // Tắt một số cảnh báo an toàn của trình biên dịch Visual Studio
#define _USE_MATH_DEFINES       // Cho phép sử dụng các hằng số toán học như M_PI
#include <SDL.h>                // Thư viện đồ họa SDL2
#include <SDL_ttf.h>            // Thư viện SDL2 để làm việc với font chữ TrueType
#include <stdio.h>              // Thư viện vào/ra chuẩn
#include <stdlib.h>             // Thư viện chuẩn (cấp phát bộ nhớ, số ngẫu nhiên, v.v.)
#include <string.h>             // Thư viện xử lý chuỗi
#include <stdbool.h>            // Cho phép sử dụng kiểu bool (true/false)
#include <limits.h>             // Định nghĩa các hằng số giới hạn (ví dụ: INT_MAX)
#include <math.h>               // Thư viện toán học
#include <ctype.h>              // Thư viện kiểm tra và chuyển đổi ký tự

// Các hằng số định nghĩa kích thước và giới hạn
#define MAX_LABEL_LENGTH 100    // Độ dài tối đa của nhãn (ví dụ: cho nút)
#define MAX_PATH_LENGTH 260     // Độ dài tối đa của đường dẫn file

// Kích thước màn hình và các yếu tố giao diện
const int SCREEN_WIDTH = 1280;  // Chiều rộng màn hình
const int SCREEN_HEIGHT = 720; // Chiều cao màn hình
const int NODE_RADIUS = 20;     // Bán kính của đỉnh đồ thị
const int BUTTON_WIDTH = 220;   // Chiều rộng mặc định của nút
const int BUTTON_HEIGHT = 50;  // Chiều cao mặc định của nút
const int PANEL_PADDING = 20;   // Khoảng đệm cho panel điều khiển

// Thuộc tính của mũi tên trên cạnh đồ thị
const int ARROW_HEAD_LENGTH = 10;                                // Độ dài đầu mũi tên
const float ARROW_HEAD_ANGLE_RAD = (float)(25.0 * M_PI / 180.0); // Góc của đầu mũi tên (radian)

// Định nghĩa màu sắc sử dụng trong ứng dụng
const SDL_Color WHITE = { 255, 255, 255, 255 };       // Màu trắng
const SDL_Color BLACK = { 0, 0, 0, 255 };             // Màu đen
const SDL_Color BLUE = { 0, 0, 255, 255 };            // Màu xanh dương
const SDL_Color RED = { 255, 0, 0, 255 };             // Màu đỏ
const SDL_Color GREEN_BTN = { 0, 200, 0, 255 };       // Màu xanh lá cây cho nút
const SDL_Color GREEN_PATH = { 0, 180, 0, 255 };      // Màu xanh lá cây cho đường đi
const SDL_Color ORANGE = { 255, 165, 0, 255 };        // Màu cam
const SDL_Color LIGHT_BLUE = { 173, 216, 230, 255 };  // Màu xanh dương nhạt
const SDL_Color GRAY = { 200, 200, 200, 255 };        // Màu xám
const SDL_Color STEP_HIGHLIGHT_NODE = { 255, 105, 180, 255 }; // Màu hồng để tô sáng đỉnh đang xử lý (step-by-step)

// Cấu trúc lưu trữ tọa độ (x, y) của một đỉnh đồ thị
typedef struct {
    int x, y;
} Node;

// Cấu trúc đại diện cho một nút bấm trên giao diện
typedef struct {
    SDL_Rect rect;                  // Hình chữ nhật đại diện cho vị trí và kích thước nút
    char label[MAX_LABEL_LENGTH];   // Nhãn của nút
    SDL_Color color;                // Màu nền của nút
    SDL_Color textColor;            // Màu chữ của nút
} Button;

// Enum định nghĩa chế độ hiển thị kết quả thuật toán
typedef enum {
    INSTANT_RESULT, // Hiển thị kết quả ngay lập tức
    STEP_BY_STEP    // Hiển thị từng bước thực thi thuật toán
} DisplayMode;

// Enum định nghĩa loại thuật toán đang được sử dụng hoặc chọn
typedef enum {
    NONE_ALGO,      // Không có thuật toán nào được chọn/chạy
    DIJKSTRA,       // Thuật toán Dijkstra
    BELLMAN_FORD    // Thuật toán Bellman-Ford
} AlgorithmType;

// Cấu trúc lưu một cặp đỉnh, dùng để đánh dấu cạnh cần tô sáng
typedef struct {
    int u, v; // Đỉnh u và đỉnh v của cạnh
} EdgePair;

// Enum định nghĩa các trạng thái khác nhau của ứng dụng
typedef enum {
    APP_STATE_MAIN_MENU,                // Trạng thái màn hình chính
    APP_STATE_GRAPH_INPUT_METHOD,       // Trạng thái chọn phương thức nhập đồ thị
    APP_STATE_DISPLAY_MODE_SELECTION,   // Trạng thái chọn chế độ hiển thị
    APP_STATE_VISUALIZATION,            // Trạng thái hiển thị đồ thị và thuật toán
    APP_STATE_TEXT_INPUT_MODAL          // Trạng thái hiển thị hộp thoại nhập văn bản
} AppState;

// Enum định nghĩa ngữ cảnh cho hộp thoại nhập văn bản
typedef enum {
    TEXT_INPUT_IDLE,                    // Không có hoạt động nhập văn bản
    TEXT_INPUT_AWAITING_FILENAME,       // Đang chờ nhập tên file đồ thị
    TEXT_INPUT_AWAITING_OUTPUT_FILENAME,// Đang chờ nhập tên file xuất kết quả
    TEXT_INPUT_AWAITING_NUM_NODES,      // Đang chờ nhập số lượng đỉnh
    TEXT_INPUT_AWAITING_MATRIX_ROW      // Đang chờ nhập một hàng của ma trận kề
} TextInputContextID;

// Các biến toàn cục
TextInputContextID currentTextInputContext = TEXT_INPUT_IDLE; // Ngữ cảnh hiện tại của việc nhập văn bản
char globalTextInputBuffer[MAX_PATH_LENGTH * 2]; // Bộ đệm chung cho việc nhập văn bản
char textInputPrompt[MAX_PATH_LENGTH];          // Chuỗi nhắc nhở cho hộp thoại nhập văn bản
AppState textInputPreviousState;                // Trạng thái ứng dụng trước khi mở hộp thoại nhập văn bản
int gui_input_N = 0;                            // Số lượng đỉnh N khi nhập từ giao diện
int gui_input_currentRow = 0;                   // Hàng hiện tại đang nhập cho ma trận kề từ giao diện

int N = 0;                                  // Số lượng đỉnh của đồ thị hiện tại
int** graph = NULL;                         // Ma trận kề của đồ thị (graph[i][j] là trọng số cạnh từ i đến j)
Node* node_positions = NULL;                // Mảng lưu tọa độ các đỉnh trên màn hình
int node_positions_count = 0;               // Số lượng đỉnh trong mảng node_positions (thường bằng N)

int* prev_nodes_for_path_build = NULL;      // Mảng lưu đỉnh trước đó trong đường đi ngắn nhất (dùng để xây dựng đường đi)
int prev_nodes_for_path_build_count = 0;    // Kích thước của mảng prev_nodes_for_path_build
int* final_path = NULL;                     // Mảng lưu các đỉnh trong đường đi ngắn nhất cuối cùng
int final_path_count = 0;                   // Số lượng đỉnh trong final_path

char graphFilePath[MAX_PATH_LENGTH] = "DOTHI.txt"; // Đường dẫn file đồ thị mặc định
int totalDistance = INT_MAX;                // Tổng khoảng cách của đường đi ngắn nhất tìm được
bool graphLoaded = false;                   // Cờ báo hiệu đồ thị đã được tải hay chưa
int startNode = -1, endNode = -1;           // Đỉnh bắt đầu và đỉnh kết thúc cho thuật toán tìm đường đi

DisplayMode currentDisplayMode = INSTANT_RESULT; // Chế độ hiển thị hiện tại
AlgorithmType activeStepAlgorithm = NONE_ALGO;   // Thuật toán đang được chạy từng bước (nếu có)
bool stepExecutionActive = false;           // Cờ báo hiệu có đang trong quá trình chạy từng bước hay không

// Biến trạng thái cho thuật toán Dijkstra chạy từng bước
int currentDijkstraStepNode = -1;           // Đỉnh hiện tại đang được xét trong Dijkstra từng bước
int* dijkstraStepDist = NULL;               // Mảng khoảng cách tạm thời trong Dijkstra từng bước
bool* dijkstraStepVisited = NULL;           // Mảng đánh dấu các đỉnh đã thăm trong Dijkstra từng bước
int* dijkstraStepPrev = NULL;               // Mảng lưu đỉnh trước đó trong Dijkstra từng bước

// Biến trạng thái cho thuật toán Bellman-Ford chạy từng bước
int currentBellmanIteration = 0;            // Số vòng lặp hiện tại của Bellman-Ford từng bước
bool bellmanNegativeCycleDetected = false;  // Cờ báo hiệu phát hiện chu trình âm trong Bellman-Ford
int* bellmanStepDist = NULL;                // Mảng khoảng cách tạm thời trong Bellman-Ford từng bước
int* bellmanStepPrev = NULL;                // Mảng lưu đỉnh trước đó trong Bellman-Ford từng bước

EdgePair* edgesToHighlight = NULL;          // Mảng các cạnh cần được tô sáng (ví dụ: cạnh đang được relax)
int edgesToHighlight_count = 0;             // Số lượng cạnh trong mảng edgesToHighlight
int edgesToHighlight_capacity = 0;          // Sức chứa hiện tại của mảng edgesToHighlight
int nodeToHighlight = -1;                   // Đỉnh cần được tô sáng đặc biệt (ví dụ: đỉnh u trong Dijkstra, đỉnh v được relax trong Bellman-Ford)

AppState currentAppState = APP_STATE_MAIN_MENU; // Trạng thái hiện tại của ứng dụng
char currentErrorMessage[MAX_PATH_LENGTH * 2] = ""; // Chuỗi lưu thông báo lỗi hiện tại để hiển thị trên giao diện

// Khai báo các nút bấm giao diện
Button startButton_MainMenu;
Button exitButton_MainMenu;
Button fromFileButton_GraphInput;
Button fromInputButton_GraphInput;
Button backButton_GraphInput;
Button instantButton_DisplayMode;
Button stepButton_DisplayMode;
Button backButton_DisplayMode;
Button runDijkstraButton_Vis;
Button runBellmanFordButton_Vis;
Button nextStepButton_Vis;
Button resetButtonUI_Vis;
Button exportButton_Vis;
Button backButton_Visualization;

// Khai báo các hàm sẽ được định nghĩa sau
void cleanup_globals(); // Dọn dẹp các tài nguyên toàn cục
bool readGraphFromFile(const char* filename); // Đọc đồ thị từ file
void drawCircle(SDL_Renderer* renderer, int cx, int cy, int r, SDL_Color color); // Vẽ hình tròn
void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color, bool centerX, int maxWidth); // Vẽ văn bản
void drawGraph(SDL_Renderer* renderer, TTF_Font* nodeFont, TTF_Font* statusFont); // Vẽ đồ thị lên màn hình
void dijkstra_instant(int start_node, int end_node); // Chạy Dijkstra và cho kết quả ngay
void bellmanFord_instant(int start_node, int end_node); // Chạy Bellman-Ford và cho kết quả ngay
void buildPath(int start_node, int end_node, const int* prev_param, int prev_param_count); // Xây dựng đường đi từ mảng prev
void exportResultsToFile(const char* filename, int startNodeParam, int endNodeParam, int distParam, const int* pathParam, int pathCountParam, bool negCycleDetected, AlgorithmType algoType); // Xuất kết quả ra file
void initDijkstraStep(int start_node); // Khởi tạo trạng thái cho Dijkstra chạy từng bước
bool performDijkstraStep(int end_node); // Thực hiện một bước của Dijkstra
void initBellmanFordStep(int start_node); // Khởi tạo trạng thái cho Bellman-Ford chạy từng bước
bool performBellmanFordStep(int end_node); // Thực hiện một bước của Bellman-Ford
void resetAlgorithmState(); // Đặt lại trạng thái của thuật toán
void reverse_int_array(int* arr, int size); // Đảo ngược một mảng số nguyên
void Button_init(Button* button, SDL_Rect rect, const char* label, SDL_Color color, SDL_Color textColor); // Khởi tạo nút bấm
void Button_draw(Button* button, SDL_Renderer* renderer, TTF_Font* font); // Vẽ nút bấm
bool Button_isClicked(Button* button, int x, int y); // Kiểm tra nút có được click không
void free_graph_matrix(); // Giải phóng bộ nhớ ma trận đồ thị
bool allocate_graph_matrix(int num_nodes); // Cấp phát bộ nhớ cho ma trận đồ thị
void add_edge_to_highlight(int u_node, int v_node); // Thêm cạnh vào danh sách tô sáng
void clear_edges_to_highlight(); // Xóa danh sách cạnh tô sáng
void drawEdgeLine(SDL_Renderer* renderer, TTF_Font* font, int x1_center, int y1_center, int x2_center, int y2_center, int weight, SDL_Color color, bool drawArrow, float text_offset_multiplier); // Vẽ một cạnh (đường thẳng) giữa hai đỉnh

void start_text_input_modal(const char* prompt_text, AppState previous_state_param, TextInputContextID context_id_for_callback); // Bắt đầu hộp thoại nhập văn bản
void process_completed_text_input(bool success, SDL_Window* window_ptr_for_msgbox); // Xử lý văn bản sau khi nhập xong

// Hàm khởi tạo một nút bấm
void Button_init(Button* button, SDL_Rect rect, const char* label, SDL_Color color, SDL_Color textColor) {
    button->rect = rect; // Gán vị trí và kích thước
    strncpy(button->label, label, MAX_LABEL_LENGTH - 1); // Sao chép nhãn
    button->label[MAX_LABEL_LENGTH - 1] = '\0'; // Đảm bảo kết thúc chuỗi
    button->color = color; // Gán màu nền
    button->textColor = textColor; // Gán màu chữ
}

// Hàm vẽ một nút bấm lên màn hình
void Button_draw(Button* button, SDL_Renderer* renderer, TTF_Font* font) {
    // Vẽ hình chữ nhật nền của nút
    SDL_SetRenderDrawColor(renderer, button->color.r, button->color.g, button->color.b, button->color.a);
    SDL_RenderFillRect(renderer, &button->rect);
    // Vẽ viền đen cho nút
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &button->rect);

    // Vẽ nhãn của nút nếu có
    if (strlen(button->label) > 0 && font) {
        SDL_Surface* surface = TTF_RenderText_Solid(font, button->label, button->textColor); // Tạo surface từ text
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // Tạo texture từ surface
            if (texture) {
                // Canh giữa text trong nút
                int textX = button->rect.x + (button->rect.w - surface->w) / 2;
                int textY = button->rect.y + (button->rect.h - surface->h) / 2;
                SDL_Rect dst = { textX, textY, surface->w, surface->h };
                SDL_RenderCopy(renderer, texture, NULL, &dst); // Vẽ texture lên renderer
                SDL_DestroyTexture(texture); // Hủy texture
            }
            SDL_FreeSurface(surface); // Giải phóng surface
        }
        else {
            fprintf(stderr, "TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        }
    }
}

// Hàm kiểm tra xem tọa độ (x, y) có nằm trong phạm vi của nút bấm không
bool Button_isClicked(Button* button, int x, int y) {
    return x >= button->rect.x && x <= button->rect.x + button->rect.w &&
        y >= button->rect.y && y <= button->rect.y + button->rect.h;
}

// Hàm giải phóng bộ nhớ đã cấp phát cho ma trận đồ thị
void free_graph_matrix() {
    if (graph) {
        // Xác định kích thước cần giải phóng, ưu tiên N nếu N > 0
        // Nếu không, thử dùng node_positions_count. Điều này có thể cần xem xét kỹ hơn
        // để đảm bảo tính nhất quán giữa cấp phát và giải phóng.
        int size_to_free = N > 0 ? N : node_positions_count;
        // Trường hợp đặc biệt nếu node_positions_count = 0 nhưng N > 0 (ví dụ, sau khi nhập N nhưng trước khi đọc file)
        if (size_to_free == 0 && N > 0) size_to_free = N;

        for (int i = 0; i < size_to_free; ++i) {
            if (graph[i]) free(graph[i]); // Giải phóng từng hàng
        }
        free(graph); // Giải phóng mảng các con trỏ hàng
        graph = NULL;
    }
}

// Hàm cấp phát bộ nhớ cho ma trận đồ thị với số lượng đỉnh cho trước
bool allocate_graph_matrix(int num_nodes) {
    free_graph_matrix(); // Giải phóng ma trận cũ nếu có
    if (num_nodes <= 0) {
        N = 0; // Đặt N = 0 nếu số đỉnh không hợp lệ
        sprintf(currentErrorMessage, "Loi: So dinh phai > 0."); // Thông báo lỗi
        return false;
    }

    // Cấp phát mảng các con trỏ hàng
    graph = (int**)malloc(num_nodes * sizeof(int*));
    if (!graph) {
        fprintf(stderr, "Failed to allocate graph rows\n");
        N = 0;
        sprintf(currentErrorMessage, "Loi cap phat hang ma tran.");
        return false;
    }
    // Cấp phát từng hàng (mảng các cột) và khởi tạo giá trị 0
    for (int i = 0; i < num_nodes; ++i) {
        graph[i] = (int*)calloc(num_nodes, sizeof(int)); // calloc sẽ khởi tạo các phần tử bằng 0
        if (!graph[i]) {
            fprintf(stderr, "Failed to allocate graph column for row %d\n", i);
            // Nếu thất bại, giải phóng các hàng đã cấp phát trước đó
            for (int k = 0; k < i; ++k) free(graph[k]);
            free(graph);
            graph = NULL;
            N = 0;
            sprintf(currentErrorMessage, "Loi cap phat cot ma tran %d.", i);
            return false;
        }
    }
    N = num_nodes; // Cập nhật số đỉnh N
    return true;
}

// Hàm thêm một cạnh (u, v) vào danh sách các cạnh cần tô sáng
void add_edge_to_highlight(int u_node, int v_node) {
    // Nếu danh sách đầy, tăng gấp đôi sức chứa
    if (edgesToHighlight_count >= edgesToHighlight_capacity) {
        edgesToHighlight_capacity = (edgesToHighlight_capacity == 0) ? 8 : edgesToHighlight_capacity * 2;
        EdgePair* new_data = (EdgePair*)realloc(edgesToHighlight, edgesToHighlight_capacity * sizeof(EdgePair));
        if (!new_data && edgesToHighlight_capacity > 0) { // Kiểm tra realloc thất bại
            fprintf(stderr, "Failed to reallocate edgesToHighlight\n");
            if (edgesToHighlight) free(edgesToHighlight); // Giải phóng bộ nhớ cũ nếu có
            edgesToHighlight = NULL;
            edgesToHighlight_count = 0;
            edgesToHighlight_capacity = 0;
            sprintf(currentErrorMessage, "Loi cap phat highlight canh.");
            return;
        }
        edgesToHighlight = new_data;
    }
    // Thêm cạnh vào danh sách
    if (edgesToHighlight) {
        edgesToHighlight[edgesToHighlight_count].u = u_node;
        edgesToHighlight[edgesToHighlight_count].v = v_node;
        edgesToHighlight_count++;
    }
}

// Hàm xóa tất cả các cạnh khỏi danh sách tô sáng
void clear_edges_to_highlight() {
    edgesToHighlight_count = 0; // Chỉ cần đặt lại số lượng về 0, không cần giải phóng bộ nhớ
}

// Hàm đọc dữ liệu đồ thị từ file
bool readGraphFromFile(const char* filename) {
    FILE* file = fopen(filename, "r"); // Mở file ở chế độ đọc
    if (!file) {
        fprintf(stderr, "Error: Cannot open graph file %s\n", filename);
        sprintf(currentErrorMessage, "Loi: Khong the mo file %s", filename);
        return false;
    }

    int tempN_file; // Biến tạm để đọc số đỉnh từ file
    // Đọc số đỉnh N
    if (fscanf(file, "%d", &tempN_file) != 1) {
        fprintf(stderr, "Error: Invalid number of nodes in file.\n");
        sprintf(currentErrorMessage, "Loi: So dinh khong hop le trong file.");
        fclose(file);
        return false;
    }
    // Kiểm tra số đỉnh có hợp lệ không
    if (tempN_file <= 0) {
        fprintf(stderr, "Error: Number of nodes must be positive.\n");
        sprintf(currentErrorMessage, "Loi: So dinh phai > 0.");
        fclose(file);
        return false;
    }

    cleanup_globals(); // Dọn dẹp trạng thái cũ trước khi đọc đồ thị mới
    // Cấp phát ma trận đồ thị mới
    if (!allocate_graph_matrix(tempN_file)) {
        fclose(file);
        return false; // allocate_graph_matrix đã set currentErrorMessage
    }

    // Đọc ma trận kề từ file
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (fscanf(file, "%d", &graph[i][j]) != 1) {
                fprintf(stderr, "Error: Invalid data in graph matrix in file.\n");
                sprintf(currentErrorMessage, "Loi: Du lieu ma tran khong hop le.");
                fclose(file);
                cleanup_globals(); // Dọn dẹp nếu có lỗi
                return false;
            }
        }
    }
    fclose(file); // Đóng file sau khi đọc xong

    // Cấp phát bộ nhớ cho mảng lưu vị trí các đỉnh
    node_positions = (Node*)malloc(N * sizeof(Node));
    if (!node_positions) {
        fprintf(stderr, "Error: Failed to allocate memory for node positions.\n");
        sprintf(currentErrorMessage, "Loi: Khong cap phat duoc cho vi tri dinh.");
        cleanup_globals();
        return false;
    }
    node_positions_count = N; // Cập nhật số lượng đỉnh

    // Tính toán vị trí các đỉnh để vẽ trên màn hình (xếp thành vòng tròn)
    int graphDisplayCenterX = SCREEN_WIDTH / 2 + (BUTTON_WIDTH + PANEL_PADDING) / 2; // Tâm X của khu vực vẽ đồ thị
    int graphDisplayCenterY = SCREEN_HEIGHT / 2 - 20; // Tâm Y (điều chỉnh một chút lên trên)
    int availableWidthForGraph = SCREEN_WIDTH - (BUTTON_WIDTH + PANEL_PADDING * 3); // Chiều rộng khả dụng
    int availableHeightForGraph = SCREEN_HEIGHT - PANEL_PADDING * 2 - 60; // Chiều cao khả dụng (trừ padding và không gian cho status)
    int R_layout = (availableWidthForGraph / 2 < availableHeightForGraph / 2 ? availableWidthForGraph / 2 : availableHeightForGraph / 2) - NODE_RADIUS; // Bán kính của vòng tròn xếp đỉnh
    R_layout = (R_layout > (N > 1 ? 80 : 0) ? R_layout : (N > 1 ? 80 : 0)); // Đảm bảo bán kính tối thiểu nếu N > 1
    if (N == 1) R_layout = 0; // Nếu chỉ có 1 đỉnh, đặt ở tâm

    for (int i = 0; i < N; ++i) {
        // Tính góc cho từng đỉnh, bắt đầu từ đỉnh trên cùng (-PI/2)
        double angle = (N == 0 || N == 1) ? -M_PI / 2.0 : (2 * M_PI * i / N - M_PI / 2.0);
        node_positions[i].x = graphDisplayCenterX + (int)(R_layout * cos(angle));
        node_positions[i].y = graphDisplayCenterY + (int)(R_layout * sin(angle));
    }

    graphLoaded = true; // Đánh dấu đồ thị đã được tải thành công
    strcpy(currentErrorMessage, ""); // Xóa thông báo lỗi cũ
    return true;
}

// Hàm vẽ một hình tròn đặc
void drawCircle(SDL_Renderer* renderer, int cx, int cy, int r, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); // Đặt màu vẽ
    // Thuật toán vẽ hình tròn cơ bản (Midpoint circle algorithm có thể hiệu quả hơn, nhưng đây là cách đơn giản)
    for (int w = 0; w < r * 2; w++) {
        for (int h = 0; h < r * 2; h++) {
            int dx = r - w; // Tọa độ x tương đối so với tâm
            int dy = r - h; // Tọa độ y tương đối so với tâm
            if (dx * dx + dy * dy <= r * r) { // Kiểm tra điểm có nằm trong hình tròn không
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy); // Vẽ điểm
            }
        }
    }
}

// Hàm vẽ văn bản lên màn hình
void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color, bool centerX, int maxWidth) {
    if (!font || !text || strlen(text) == 0) return; // Kiểm tra đầu vào hợp lệ

    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color); // Tạo surface từ văn bản
    if (!surface) {
        fprintf(stderr, "TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // Tạo texture từ surface
    if (!texture) {
        SDL_FreeSurface(surface);
        fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        return;
    }

    int textW = surface->w;
    int textH = surface->h;
    SDL_Rect dst = { x, y, textW, textH }; // Hình chữ nhật đích để vẽ text

    if (centerX) { // Nếu cần căn giữa theo chiều ngang
        dst.x = x - textW / 2;
    }

    // Nếu chiều rộng tối đa được chỉ định và text dài hơn, cắt bớt
    if (maxWidth > 0 && textW > maxWidth) {
        SDL_Rect srcClip = { 0, 0, (int)((float)maxWidth / textW * surface->w), surface->h }; // Phần của source texture để copy
        dst.w = maxWidth; // Giới hạn chiều rộng vẽ
        SDL_RenderCopy(renderer, texture, &srcClip, &dst);
    }
    else {
        SDL_RenderCopy(renderer, texture, NULL, &dst); // Vẽ toàn bộ texture
    }

    SDL_FreeSurface(surface); // Giải phóng surface
    SDL_DestroyTexture(texture); // Giải phóng texture
}

// Hàm vẽ một cạnh (đường thẳng) giữa hai đỉnh, có thể kèm mũi tên và trọng số
void drawEdgeLine(SDL_Renderer* renderer, TTF_Font* font,
    int x1_center, int y1_center, // Tọa độ tâm đỉnh 1
    int x2_center, int y2_center, // Tọa độ tâm đỉnh 2
    int weight, SDL_Color color, bool drawArrow, float text_offset_multiplier) {

    char buffer[32]; // Buffer cho trọng số
    // Tính vector hướng từ đỉnh 1 đến đỉnh 2
    double dx_nodes = (double)x2_center - x1_center;
    double dy_nodes = (double)y2_center - y1_center;
    double dist_center_to_center = sqrt(dx_nodes * dx_nodes + dy_nodes * dy_nodes); // Khoảng cách giữa tâm hai đỉnh

    if (dist_center_to_center < 1e-6) return; // Nếu hai đỉnh trùng nhau, không vẽ

    // Tính điểm bắt đầu và kết thúc của đường thẳng trên_viền_ của các đỉnh (không phải tâm)
    int x_line_start = x1_center + (int)(NODE_RADIUS * dx_nodes / dist_center_to_center);
    int y_line_start = y1_center + (int)(NODE_RADIUS * dy_nodes / dist_center_to_center);
    int x_line_end = x2_center - (int)(NODE_RADIUS * dx_nodes / dist_center_to_center);
    int y_line_end = y2_center - (int)(NODE_RADIUS * dy_nodes / dist_center_to_center);

    // Vẽ đường thẳng chính của cạnh
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(renderer, x_line_start, y_line_start, x_line_end, y_line_end);

    // Tính vector và độ dài của đoạn thẳng vừa vẽ (từ viền tới viền)
    double dx_segment = (double)x_line_end - x_line_start;
    double dy_segment = (double)y_line_end - y_line_start;
    double line_segment_length = sqrt(dx_segment * dx_segment + dy_segment * dy_segment);

    // Vẽ mũi tên nếu được yêu cầu và đoạn thẳng đủ dài
    if (drawArrow && line_segment_length > ARROW_HEAD_LENGTH * 0.5) {
        double line_angle = atan2(dy_segment, dx_segment); // Góc của đoạn thẳng
        // Tính tọa độ hai điểm của đầu mũi tên
        int ah_p1_x = x_line_end - (int)(ARROW_HEAD_LENGTH * cos(line_angle - ARROW_HEAD_ANGLE_RAD));
        int ah_p1_y = y_line_end - (int)(ARROW_HEAD_LENGTH * sin(line_angle - ARROW_HEAD_ANGLE_RAD));
        int ah_p2_x = x_line_end - (int)(ARROW_HEAD_LENGTH * cos(line_angle + ARROW_HEAD_ANGLE_RAD));
        int ah_p2_y = y_line_end - (int)(ARROW_HEAD_LENGTH * sin(line_angle + ARROW_HEAD_ANGLE_RAD));
        // Vẽ hai nét của đầu mũi tên
        SDL_RenderDrawLine(renderer, x_line_end, y_line_end, ah_p1_x, ah_p1_y);
        SDL_RenderDrawLine(renderer, x_line_end, y_line_end, ah_p2_x, ah_p2_y);
    }

    // Vẽ trọng số của cạnh
    sprintf(buffer, "%d", weight); // Chuyển trọng số sang chuỗi
    int text_w, text_h;
    TTF_SizeText(font, buffer, &text_w, &text_h); // Lấy kích thước của text trọng số
    // Vị trí neo của text (giữa đoạn thẳng)
    int text_anchor_x = (x_line_start + x_line_end) / 2;
    int text_anchor_y = (y_line_start + y_line_end) / 2;
    int text_offset_dist = 12; // Khoảng cách dịch chuyển text ra khỏi đường thẳng
    int text_draw_x = text_anchor_x;
    int text_draw_y = text_anchor_y;

    // Dịch chuyển text ra một bên của đường thẳng để tránh chồng chéo
    // Sử dụng vector pháp tuyến của đoạn thẳng để dịch chuyển
    if (line_segment_length > 1e-6) {
        double norm_line_dx = dx_segment / line_segment_length; // dx chuẩn hóa
        double norm_line_dy = dy_segment / line_segment_length; // dy chuẩn hóa
        // Vector pháp tuyến (-dy, dx) hoặc (dy, -dx)
        text_draw_x += (int)(text_offset_dist * (-norm_line_dy) * text_offset_multiplier);
        text_draw_y += (int)(text_offset_dist * norm_line_dx * text_offset_multiplier);
    }
    else { // Trường hợp đặc biệt, có thể là cạnh vòng (self-loop), nhưng ở đây không xử lý phức tạp
        text_draw_x = x1_center + NODE_RADIUS + text_w / 2 + 2;
        text_draw_y = y1_center - NODE_RADIUS - text_h / 2 - 2;
    }
    // Vẽ text trọng số (canh giữa tại điểm text_draw_x, text_draw_y)
    drawText(renderer, font, buffer, text_draw_x - text_w / 2, text_draw_y - text_h / 2, BLACK, false, 0);
}


// Hàm vẽ toàn bộ đồ thị lên màn hình
void drawGraph(SDL_Renderer* renderer, TTF_Font* nodeFont, TTF_Font* statusFont) {
    if (!graphLoaded || N == 0 || !node_positions) return; // Kiểm tra đồ thị đã tải chưa
    char buffer[256]; // Buffer cho các chuỗi text

    // Vẽ các cạnh
    // Duyệt qua nửa trên của ma trận kề để tránh vẽ mỗi cạnh hai lần (đối với cạnh vô hướng)
    // hoặc để xử lý riêng cạnh i->j và j->i (đối với cạnh có hướng)
    for (int i = 0; i < N; ++i) {
        for (int j = i + 1; j < N; ++j) { // Chỉ xét j > i
            bool edge_ij_exists = (graph[i][j] != 0); // Cạnh từ i đến j có tồn tại không?
            bool edge_ji_exists = (graph[j][i] != 0); // Cạnh từ j đến i có tồn tại không?

            SDL_Color color_ij = BLACK; // Màu mặc định cho cạnh i->j
            SDL_Color color_ji = BLACK; // Màu mặc định cho cạnh j->i
            bool ij_is_final = false; // Cạnh i->j có nằm trong đường đi cuối cùng không?
            bool ji_is_final = false; // Cạnh j->i có nằm trong đường đi cuối cùng không?

            // Kiểm tra xem cạnh có nằm trong đường đi cuối cùng không
            for (int k = 1; k < final_path_count; ++k) {
                if (final_path[k - 1] == i && final_path[k] == j) {
                    color_ij = GREEN_PATH; ij_is_final = true;
                }
                if (final_path[k - 1] == j && final_path[k] == i) {
                    color_ji = GREEN_PATH; ji_is_final = true;
                }
            }
            // Nếu đang chạy từng bước, kiểm tra xem cạnh có cần tô sáng không
            if (stepExecutionActive) {
                for (int k = 0; k < edgesToHighlight_count; ++k) {
                    if (edgesToHighlight[k].u == i && edgesToHighlight[k].v == j && !ij_is_final) {
                        color_ij = LIGHT_BLUE; // Tô sáng cạnh i->j
                    }
                    if (edgesToHighlight[k].u == j && edgesToHighlight[k].v == i && !ji_is_final) {
                        color_ji = LIGHT_BLUE; // Tô sáng cạnh j->i
                    }
                }
            }

            // Xử lý trường hợp cạnh hai chiều (vô hướng) hoặc hai cạnh một chiều đối xứng
            if (edge_ij_exists && edge_ji_exists && graph[i][j] == graph[j][i]) {
                // Nếu trọng số bằng nhau, coi như cạnh vô hướng, vẽ một đường không mũi tên
                SDL_Color display_color = ij_is_final || ji_is_final ? GREEN_PATH :
                    (color_ij.r == LIGHT_BLUE.r && color_ij.g == LIGHT_BLUE.g && color_ij.b == LIGHT_BLUE.b ? LIGHT_BLUE : BLACK);
                drawEdgeLine(renderer, nodeFont,
                    node_positions[i].x, node_positions[i].y,
                    node_positions[j].x, node_positions[j].y,
                    graph[i][j], display_color, false, 1.0f); // false: không vẽ mũi tên
            }
            else {
                // Xử lý các cạnh một chiều riêng biệt
                if (edge_ij_exists) {
                    // text_offset_multiplier để dịch chuyển text trọng số nếu có cả hai cạnh i->j và j->i
                    // để chúng không đè lên nhau
                    drawEdgeLine(renderer, nodeFont,
                        node_positions[i].x, node_positions[i].y,
                        node_positions[j].x, node_positions[j].y,
                        graph[i][j], color_ij, true, (edge_ji_exists ? 1.0f : 1.0f)); // true: vẽ mũi tên
                }
                if (edge_ji_exists) {
                    drawEdgeLine(renderer, nodeFont,
                        node_positions[j].x, node_positions[j].y,
                        node_positions[i].x, node_positions[i].y,
                        graph[j][i], color_ji, true, (edge_ij_exists ? -1.0f : 1.0f));
                }
            }
        }
    }

    // Vẽ các đỉnh
    for (int i = 0; i < N; ++i) {
        SDL_Color nodeColor = BLUE; // Màu mặc định cho đỉnh
        bool isFinalPathNode = false; // Đỉnh có nằm trong đường đi cuối cùng không?
        for (int k = 0; k < final_path_count; ++k) if (final_path[k] == i) { isFinalPathNode = true; break; }

        // Tô màu đặc biệt cho đỉnh tùy theo trạng thái thuật toán từng bước
        if (stepExecutionActive) {
            if (activeStepAlgorithm == DIJKSTRA) {
                // Nếu đỉnh đã thăm (và không phải là đỉnh cuối của đường đi, không phải start/end) thì tô màu xanh nhạt
                if (dijkstraStepVisited && i < N && dijkstraStepVisited[i] && !isFinalPathNode && i != startNode && i != endNode) nodeColor = LIGHT_BLUE;
                // Nếu đỉnh là đỉnh đang xét (u) thì tô màu hồng
                if (i == currentDijkstraStepNode && !isFinalPathNode) nodeColor = STEP_HIGHLIGHT_NODE;
            }
            else if (activeStepAlgorithm == BELLMAN_FORD) {
                // Nếu đỉnh là đỉnh được tô sáng (ví dụ, đỉnh v vừa được relax) thì tô màu hồng
                if (i == nodeToHighlight && !isFinalPathNode && i != startNode && i != endNode) nodeColor = STEP_HIGHLIGHT_NODE;
            }
        }

        if (isFinalPathNode) nodeColor = GREEN_PATH; // Đỉnh thuộc đường đi cuối cùng
        if (i == startNode) nodeColor = RED; // Đỉnh bắt đầu
        if (i == endNode && i != startNode) nodeColor = ORANGE; // Đỉnh kết thúc

        drawCircle(renderer, node_positions[i].x, node_positions[i].y, NODE_RADIUS, nodeColor); // Vẽ hình tròn đỉnh

        // Vẽ nhãn cho đỉnh (A, B, C, ...)
        sprintf(buffer, "%c", 'A' + i);
        int labelW, labelH;
        TTF_SizeText(nodeFont, buffer, &labelW, &labelH); // Lấy kích thước nhãn
        drawText(renderer, nodeFont, buffer, node_positions[i].x - labelW / 2, node_positions[i].y - labelH / 2, WHITE, false, 0); // Vẽ nhãn ở giữa đỉnh

        // Nếu đang chạy từng bước, hiển thị khoảng cách tạm thời (dist[]) bên cạnh đỉnh
        if (stepExecutionActive) {
            char distText[20] = "";
            int currentDistVal = INT_MAX;
            if (activeStepAlgorithm == DIJKSTRA && dijkstraStepDist && i < N) {
                currentDistVal = dijkstraStepDist[i];
            }
            else if (activeStepAlgorithm == BELLMAN_FORD && bellmanStepDist && i < N) {
                currentDistVal = bellmanStepDist[i];
            }

            if (currentDistVal != INT_MAX) sprintf(distText, "%d", currentDistVal);
            else strcpy(distText, "inf"); // "inf" cho vô cực

            if (strlen(distText) > 0) {
                int distTextW, distTextH;
                TTF_SizeText(nodeFont, distText, &distTextW, &distTextH);
                // Vẽ text khoảng cách ở góc trên bên phải của đỉnh
                drawText(renderer, nodeFont, distText, node_positions[i].x + NODE_RADIUS + 5, node_positions[i].y - NODE_RADIUS - distTextH, BLACK, false, 0);
            }
        }
    }

    // Hiển thị thông tin trạng thái thuật toán ở phía trên khu vực đồ thị
    int statusTextX = BUTTON_WIDTH + PANEL_PADDING * 2; // Vị trí X của text trạng thái
    int statusTextY = PANEL_PADDING; // Vị trí Y
    char algoStatusString[MAX_PATH_LENGTH] = "";
    if (stepExecutionActive) {
        if (activeStepAlgorithm == BELLMAN_FORD) {
            sprintf(algoStatusString, "Bellman-Ford Iter: %d/%d", currentBellmanIteration, N); // Số vòng lặp Bellman-Ford
            if (N > 0 && currentBellmanIteration == N) { // Vòng lặp thứ N là để kiểm tra chu trình âm
                strcpy(algoStatusString, "Bellman-Ford: Kiem tra chu trinh am");
            }
        }
        else if (activeStepAlgorithm == DIJKSTRA) {
            if (currentDijkstraStepNode != -1)
                sprintf(algoStatusString, "Dijkstra: Dang xet dinh %c", 'A' + currentDijkstraStepNode); // Đỉnh đang xét của Dijkstra
            else
                strcpy(algoStatusString, "Dijkstra: Bat dau");
        }
    }
    // Nếu thuật toán đã hoàn thành (không phải từng bước)
    else if (activeStepAlgorithm != NONE_ALGO && startNode != -1 && endNode != -1) {
        if (activeStepAlgorithm == DIJKSTRA) {
            bool hasNegative = false; // Kiểm tra có cạnh âm không (Dijkstra có thể sai)
            if (graphLoaded && N > 0) {
                for (int r = 0; r < N; ++r) {
                    for (int c = 0; c < N; ++c) {
                        if (graph[r][c] < 0) {
                            hasNegative = true; break;
                        }
                    }
                    if (hasNegative) break;
                }
            }
            if (hasNegative) strcpy(algoStatusString, "Dijkstra: Hoan thanh (CANH BAO: Ket qua co the sai voi canh am!)");
            else strcpy(algoStatusString, "Dijkstra: Hoan thanh");
        }
        else if (activeStepAlgorithm == BELLMAN_FORD) {
            if (bellmanNegativeCycleDetected) {
                strcpy(algoStatusString, "Bellman-Ford: Phat hien chu trinh am!");
            }
            else {
                strcpy(algoStatusString, "Bellman-Ford: Hoan thanh");
            }
        }
    }
    if (strlen(algoStatusString) > 0) {
        drawText(renderer, statusFont, algoStatusString, statusTextX, statusTextY, BLACK, false, SCREEN_WIDTH - statusTextX - PANEL_PADDING);
    }

    // Hiển thị thông tin đường đi và tổng khoảng cách ở dưới màn hình
    int bottomMessageY = SCREEN_HEIGHT - PANEL_PADDING; // Y bắt đầu của thông báo dưới cùng
    int lineHeight = 0;
    TTF_SizeText(nodeFont, "Tg", NULL, &lineHeight); // Lấy chiều cao của một dòng text
    lineHeight = lineHeight > 0 ? lineHeight + 2 : 20; // Thêm chút đệm

    if (!stepExecutionActive && final_path_count > 0 && totalDistance != INT_MAX) {
        // Hiển thị đường đi
        bottomMessageY -= lineHeight;
        char pathLabelStr[MAX_PATH_LENGTH * 2] = "Duong di ngan nhat: ";
        size_t currentLen = strlen(pathLabelStr);
        for (int k = 0; k < final_path_count; ++k) {
            char nodeCharStr[5];
            sprintf(nodeCharStr, "%c", 'A' + final_path[k]);
            // Kiểm tra tràn buffer trước khi nối chuỗi
            if (currentLen + strlen(nodeCharStr) + (k < final_path_count - 1 ? 4 : 0) < sizeof(pathLabelStr) - 1) {
                strcat(pathLabelStr, nodeCharStr);
                currentLen += strlen(nodeCharStr);
                if (k < final_path_count - 1) {
                    strcat(pathLabelStr, " -> ");
                    currentLen += 4;
                }
            }
            else {
                strcat(pathLabelStr, "..."); // Nếu quá dài, thêm dấu ...
                break;
            }
        }
        drawText(renderer, nodeFont, pathLabelStr, PANEL_PADDING, bottomMessageY, BLACK, false, SCREEN_WIDTH - 2 * PANEL_PADDING);

        // Hiển thị tổng khoảng cách
        bottomMessageY -= lineHeight;
        sprintf(buffer, "Tong quang duong: %d", totalDistance);
        drawText(renderer, nodeFont, buffer, PANEL_PADDING, bottomMessageY, BLACK, false, 0);
    }
    // Trường hợp không tìm thấy đường đi hoặc có chu trình âm
    else if (!stepExecutionActive && startNode != -1 && endNode != -1 && activeStepAlgorithm != NONE_ALGO &&
        (totalDistance == INT_MAX || bellmanNegativeCycleDetected)) {
        bottomMessageY -= lineHeight;
        if (bellmanNegativeCycleDetected && activeStepAlgorithm == BELLMAN_FORD) {
            drawText(renderer, nodeFont, "Phat hien chu trinh am!", PANEL_PADDING, bottomMessageY, RED, false, 0);
        }
        else if (totalDistance == INT_MAX && !(startNode == endNode)) { // startNode != endNode và không có đường
            drawText(renderer, nodeFont, "Khong tim thay duong di!", PANEL_PADDING, bottomMessageY, RED, false, 0);
        }
        else if (startNode == endNode) { // Trường hợp startNode == endNode, đường đi là chính nó, khoảng cách 0
            bottomMessageY -= lineHeight; // Dòng cho "Tong quang duong"
            sprintf(buffer, "Tong quang duong: 0");
            drawText(renderer, nodeFont, buffer, PANEL_PADDING, bottomMessageY, BLACK, false, 0);
            bottomMessageY -= lineHeight; // Dòng cho "Duong di ngan nhat"
            sprintf(buffer, "Duong di ngan nhat: %c", 'A' + startNode);
            drawText(renderer, nodeFont, buffer, PANEL_PADDING, bottomMessageY, BLACK, false, 0);
        }
    }
    else { // Trường hợp chưa chọn đủ đỉnh hoặc chưa chạy thuật toán
        if (startNode == -1 && graphLoaded) {
            bottomMessageY -= lineHeight;
            drawText(renderer, nodeFont, "Chon diem bat dau!", PANEL_PADDING, bottomMessageY, RED, false, 0);
        }
        else if (endNode == -1 && graphLoaded && startNode != -1) {
            bottomMessageY -= lineHeight;
            drawText(renderer, nodeFont, "Chon diem ket thuc!", PANEL_PADDING, bottomMessageY, RED, false, 0);
        }
    }
}

// Thực thi thuật toán Dijkstra và cho kết quả ngay lập tức
void dijkstra_instant(int start_node_param, int end_node_param) {
    // Kiểm tra đầu vào
    if (start_node_param < 0 || start_node_param >= N || (end_node_param != -1 && (end_node_param < 0 || end_node_param >= N)) || N == 0 || !graphLoaded) {
        totalDistance = INT_MAX;
        if (prev_nodes_for_path_build) free(prev_nodes_for_path_build);
        prev_nodes_for_path_build = NULL;
        prev_nodes_for_path_build_count = 0;
        if (final_path) free(final_path); final_path = NULL; final_path_count = 0;
        return;
    }

    // Cấp phát bộ nhớ cho các mảng phụ trợ
    int* dist_local = (int*)malloc(N * sizeof(int)); // Mảng khoảng cách
    bool* visited = (bool*)calloc(N, sizeof(bool));  // Mảng đánh dấu đỉnh đã thăm (calloc khởi tạo là false)
    if (prev_nodes_for_path_build) free(prev_nodes_for_path_build); // Giải phóng mảng prev cũ nếu có
    prev_nodes_for_path_build = (int*)malloc(N * sizeof(int)); // Mảng lưu đỉnh trước đó

    if (!dist_local || !visited || !prev_nodes_for_path_build) {
        fprintf(stderr, "Memory allocation failed in dijkstra_instant\n");
        if (dist_local) free(dist_local);
        if (visited) free(visited);
        if (prev_nodes_for_path_build) { free(prev_nodes_for_path_build); prev_nodes_for_path_build = NULL; }
        totalDistance = INT_MAX;
        prev_nodes_for_path_build_count = 0;
        if (final_path) free(final_path); final_path = NULL; final_path_count = 0;
        sprintf(currentErrorMessage, "Loi cap phat bo nho Dijkstra.");
        return;
    }
    prev_nodes_for_path_build_count = N;

    // Khởi tạo khoảng cách và mảng prev
    for (int i = 0; i < N; ++i) {
        dist_local[i] = INT_MAX;
        prev_nodes_for_path_build[i] = -1; // -1 nghĩa là chưa có đỉnh trước đó
    }
    dist_local[start_node_param] = 0; // Khoảng cách từ đỉnh bắt đầu đến chính nó là 0

    // Vòng lặp chính của Dijkstra
    for (int count = 0; count < N; ++count) {
        int u = -1, minDistVal = INT_MAX;
        // Tìm đỉnh u chưa thăm có dist_local[u] nhỏ nhất
        for (int v_node_idx = 0; v_node_idx < N; ++v_node_idx) {
            if (!visited[v_node_idx] && dist_local[v_node_idx] < minDistVal) {
                minDistVal = dist_local[v_node_idx];
                u = v_node_idx;
            }
        }

        // Nếu không tìm thấy đỉnh u (tất cả các đỉnh còn lại không thể đến được) hoặc u là đỉnh đích (nếu có) thì dừng
        if (u == -1 || dist_local[u] == INT_MAX) break;
        visited[u] = true; // Đánh dấu u đã thăm

        if (u == end_node_param && end_node_param != -1) break; // Đã đến đích, dừng sớm

        // Cập nhật khoảng cách cho các đỉnh kề v của u
        for (int v_node_idx = 0; v_node_idx < N; ++v_node_idx) {
            // Nếu có cạnh từ u đến v, v chưa thăm, và đường đi qua u ngắn hơn
            if (graph[u][v_node_idx] != 0 && !visited[v_node_idx] && dist_local[u] != INT_MAX) {
                // Sử dụng long long để tránh tràn số khi cộng
                long long new_dist_ll = (long long)dist_local[u] + graph[u][v_node_idx];
                if (new_dist_ll < dist_local[v_node_idx]) {
                    if (new_dist_ll < INT_MIN || new_dist_ll > INT_MAX) continue; // Kiểm tra tràn số int
                    dist_local[v_node_idx] = (int)new_dist_ll;
                    prev_nodes_for_path_build[v_node_idx] = u; // Ghi nhận u là đỉnh trước của v
                }
            }
        }
    }

    // Lưu tổng khoảng cách đến đỉnh đích (nếu có)
    totalDistance = (end_node_param != -1 && end_node_param < N && end_node_param >= 0) ? dist_local[end_node_param] : INT_MAX;
    if (start_node_param == end_node_param && N > 0) totalDistance = 0; // Nếu đỉnh bắt đầu và kết thúc trùng nhau

    // Giải phóng bộ nhớ tạm
    free(dist_local);
    free(visited);
    // prev_nodes_for_path_build sẽ được buildPath sử dụng và sau đó có thể được giải phóng ở cleanup_globals hoặc lần chạy tiếp theo
}

// Thực thi thuật toán Bellman-Ford và cho kết quả ngay lập tức
void bellmanFord_instant(int start_node_param, int end_node_param) {
    // Kiểm tra đầu vào
    if (start_node_param < 0 || start_node_param >= N || (end_node_param != -1 && (end_node_param < 0 || end_node_param >= N)) || N == 0 || !graphLoaded) {
        totalDistance = INT_MAX;
        bellmanNegativeCycleDetected = false;
        if (prev_nodes_for_path_build) free(prev_nodes_for_path_build);
        prev_nodes_for_path_build = NULL;
        prev_nodes_for_path_build_count = 0;
        if (final_path) free(final_path); final_path = NULL; final_path_count = 0;
        return;
    }

    // Cấp phát bộ nhớ
    int* dist_local = (int*)malloc(N * sizeof(int));
    if (prev_nodes_for_path_build) free(prev_nodes_for_path_build);
    prev_nodes_for_path_build = (int*)malloc(N * sizeof(int));

    if (!dist_local || !prev_nodes_for_path_build) {
        fprintf(stderr, "Memory allocation failed in bellmanFord_instant\n");
        if (dist_local) free(dist_local);
        if (prev_nodes_for_path_build) { free(prev_nodes_for_path_build); prev_nodes_for_path_build = NULL; }
        totalDistance = INT_MAX;
        bellmanNegativeCycleDetected = false;
        prev_nodes_for_path_build_count = 0;
        if (final_path) free(final_path); final_path = NULL; final_path_count = 0;
        sprintf(currentErrorMessage, "Loi cap phat bo nho BellmanFord.");
        return;
    }
    prev_nodes_for_path_build_count = N;

    // Khởi tạo
    for (int i = 0; i < N; ++i) {
        dist_local[i] = INT_MAX;
        prev_nodes_for_path_build[i] = -1;
    }
    dist_local[start_node_param] = 0;
    bellmanNegativeCycleDetected = false; // Reset cờ chu trình âm

    // Lặp N-1 lần để "relax" các cạnh
    for (int i = 1; i < N; ++i) { // Lặp N-1 lần
        for (int u = 0; u < N; ++u) { // Duyệt qua tất cả các đỉnh u
            if (dist_local[u] == INT_MAX) continue; // Bỏ qua nếu u chưa đến được
            for (int v = 0; v < N; ++v) { // Duyệt qua tất cả các đỉnh v
                if (graph[u][v] != 0) { // Nếu có cạnh từ u đến v
                    long long new_dist_check = (long long)dist_local[u] + graph[u][v];
                    if (new_dist_check < dist_local[v]) {
                        if (new_dist_check < INT_MIN || new_dist_check > INT_MAX) continue; // Kiểm tra tràn số
                        dist_local[v] = (int)new_dist_check;
                        prev_nodes_for_path_build[v] = u;
                    }
                }
            }
        }
    }

    // Kiểm tra chu trình âm (lặp thêm một lần nữa)
    for (int u = 0; u < N; ++u) {
        if (dist_local[u] == INT_MAX) continue;
        for (int v = 0; v < N; ++v) {
            if (graph[u][v] != 0) {
                long long new_dist_check = (long long)dist_local[u] + graph[u][v];
                if (new_dist_check < dist_local[v]) {
                    if (new_dist_check >= INT_MIN && new_dist_check <= INT_MAX) { // Chỉ xét nếu không tràn số
                        fprintf(stderr, "Phat hien chu trinh am!\n");
                        bellmanNegativeCycleDetected = true;
                        totalDistance = INT_MAX; // Không có đường đi ngắn nhất xác định
                        // Giải phóng mảng prev vì không dùng để xây dựng đường đi khi có chu trình âm
                        if (prev_nodes_for_path_build) { free(prev_nodes_for_path_build); prev_nodes_for_path_build = NULL; }
                        prev_nodes_for_path_build_count = 0;
                        if (final_path) { free(final_path); final_path = NULL; final_path_count = 0; }
                        free(dist_local);
                        return; // Thoát sớm khi phát hiện chu trình âm
                    }
                }
            }
        }
    }

    // Lưu tổng khoảng cách
    totalDistance = (end_node_param != -1 && end_node_param < N && end_node_param >= 0) ? dist_local[end_node_param] : INT_MAX;
    if (start_node_param == end_node_param && N > 0) totalDistance = 0;

    free(dist_local); // Giải phóng bộ nhớ tạm
    // prev_nodes_for_path_build sẽ được sử dụng bởi buildPath
}

// Hàm đảo ngược một mảng số nguyên
void reverse_int_array(int* arr, int size) {
    int i = 0, j = size - 1;
    while (i < j) {
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
        i++;
        j--;
    }
}

// Hàm xây dựng đường đi ngắn nhất từ mảng prev (mảng đỉnh trước đó)
void buildPath(int start_node_param, int end_node_param, const int* prev_param, int prev_param_count_unused) {
    // (void)prev_param_count_unused; // Đánh dấu là không sử dụng để tránh warning (nếu có)

    if (final_path) { // Giải phóng đường đi cũ nếu có
        free(final_path);
        final_path = NULL;
    }
    final_path_count = 0;

    // Kiểm tra đầu vào
    if (!prev_param || N == 0 || end_node_param < 0 || end_node_param >= N || start_node_param < 0 || start_node_param >= N) {
        return;
    }

    // Nếu không có đường đi (totalDistance = INT_MAX) và không phải là trường hợp đỉnh đầu cuối trùng nhau
    if (totalDistance == INT_MAX && start_node_param != end_node_param) {
        return; // Không có đường đi để xây dựng
    }

    // Trường hợp đặc biệt: đỉnh bắt đầu và kết thúc trùng nhau
    if (start_node_param == end_node_param) {
        final_path = (int*)malloc(sizeof(int));
        if (final_path) {
            final_path[0] = start_node_param;
            final_path_count = 1;
        }
        else {
            fprintf(stderr, "buildPath: Failed to allocate memory for single node path.\n");
        }
        return;
    }

    // Nếu đỉnh kết thúc không thể đến được từ đỉnh bắt đầu (không có prev)
    // và chúng không phải là cùng một đỉnh (đã xử lý ở trên)
    if (prev_param[end_node_param] == -1 && end_node_param != start_node_param) {
        return; // Không có đường đi
    }

    // Sử dụng mảng tạm để lưu đường đi ngược từ cuối về đầu
    int* tempPath = (int*)malloc(N * sizeof(int)); // Tối đa N đỉnh trong đường đi
    if (!tempPath) {
        fprintf(stderr, "Failed to allocate memory for tempPath in buildPath\n");
        return;
    }
    int currentPathCount = 0;
    int at = end_node_param; // Bắt đầu từ đỉnh kết thúc

    // Lần theo mảng prev để xây dựng đường đi ngược
    while (at != -1) { // Dừng khi gặp đỉnh không có prev (-1) hoặc đỉnh bắt đầu
        if (currentPathCount >= N) { // Kiểm tra tràn (lỗi logic nếu xảy ra)
            fprintf(stderr, "Path construction overflow (more than N nodes). Cycle in prev array at node %d?\n", at);
            currentPathCount = 0; // Reset để không tạo đường đi lỗi
            break;
        }
        tempPath[currentPathCount++] = at; // Thêm đỉnh hiện tại vào đường đi tạm

        if (at == start_node_param) { // Đã về đến đỉnh bắt đầu
            break;
        }

        // Kiểm tra chỉ số 'at' có hợp lệ không trước khi truy cập prev_param[at]
        if (at < 0 || at >= N) { // Thường không nên xảy ra nếu logic đúng
            fprintf(stderr, "Error: 'at' index %d out of bounds in buildPath.\n", at);
            currentPathCount = 0; break;
        }

        int next_at = prev_param[at]; // Lấy đỉnh trước đó

        // Kiểm tra giá trị next_at có hợp lệ không (nếu không phải -1)
        if (next_at != -1 && (next_at < 0 || next_at >= N)) {
            fprintf(stderr, "Error: Invalid 'next_at' value (%d) from prev_param[%d] in buildPath.\n", next_at, at);
            currentPathCount = 0; break;
        }
        // Kiểm tra trường hợp tự lặp (next_at == at) mà không phải là đỉnh bắt đầu (có thể là lỗi trong prev_param)
        if (next_at == at && at != start_node_param) { // Nếu at == start_node_param và prev[at] == at, đó là lỗi
            fprintf(stderr, "Error: Cycle detected in prev array (next_at == at) at node %d, not start node.\n", at);
            currentPathCount = 0; break;
        }

        at = next_at; // Di chuyển đến đỉnh trước đó
    }

    // Nếu xây dựng đường đi thành công (kết thúc ở start_node_param)
    if (currentPathCount > 0 && tempPath[currentPathCount - 1] == start_node_param) {
        // Cấp phát bộ nhớ cho final_path và sao chép đường đi
        final_path = (int*)malloc(currentPathCount * sizeof(int));
        if (!final_path) {
            fprintf(stderr, "Failed to allocate memory for final_path\n");
        }
        else {
            final_path_count = currentPathCount;
            for (int i = 0; i < currentPathCount; ++i) {
                final_path[i] = tempPath[i];
            }
            reverse_int_array(final_path, final_path_count); // Đảo ngược để có thứ tự đúng từ start đến end
        }
    }
    else {
        // Nếu không xây dựng được đường đi (ví dụ, không kết thúc ở start_node_param)
        final_path_count = 0;
    }
    free(tempPath); // Giải phóng mảng tạm
}

// Hàm xuất kết quả tìm đường đi ra file
void exportResultsToFile(const char* filename, int startNodeParam, int endNodeParam,
    int distParam, const int* pathParam, int pathCountParam,
    bool negCycleDetected, AlgorithmType algoType) {
    FILE* outFile = fopen(filename, "w"); // Mở file ở chế độ ghi
    if (!outFile) {
        fprintf(stderr, "Loi: Khong the mo file de ghi: %s\n", filename);
        char errorMsg[MAX_PATH_LENGTH + 50];
        snprintf(errorMsg, sizeof(errorMsg), "Khong the tao file: %s", filename);
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Loi Xuat File", errorMsg, NULL);
        return;
    }

    // Ghi thông tin chung
    fprintf(outFile, "Ket qua tim duong di ngan nhat:\n");
    fprintf(outFile, "===================================\n");

    // Ghi loại thuật toán
    if (algoType == DIJKSTRA) {
        fprintf(outFile, "Thuat toan: Dijkstra\n");
    }
    else if (algoType == BELLMAN_FORD) {
        fprintf(outFile, "Thuat toan: Bellman-Ford\n");
    }
    else {
        fprintf(outFile, "Thuat toan: Chua chon/Chua chay\n");
    }

    // Ghi đỉnh bắt đầu và kết thúc
    if (startNodeParam != -1) {
        fprintf(outFile, "Dinh bat dau: %c (%d)\n", 'A' + startNodeParam, startNodeParam);
    }
    else {
        fprintf(outFile, "Dinh bat dau: Chua chon\n");
    }
    if (endNodeParam != -1) {
        fprintf(outFile, "Dinh ket thuc: %c (%d)\n", 'A' + endNodeParam, endNodeParam);
    }
    else {
        fprintf(outFile, "Dinh ket thuc: Chua chon\n");
    }
    fprintf(outFile, "\n"); // Dòng trống

    // Ghi kết quả
    if (negCycleDetected && algoType == BELLMAN_FORD) { // Trường hợp có chu trình âm
        fprintf(outFile, "KET QUA: Phat hien chu trinh am!\n");
        fprintf(outFile, "Khong co duong di ngan nhat xac dinh.\n");
    }
    else if (distParam == INT_MAX) { // Trường hợp không tìm thấy đường đi (hoặc chưa chạy)
        if (startNodeParam != -1 && endNodeParam != -1) {
            if (startNodeParam == endNodeParam) { // Đường đi từ đỉnh đến chính nó
                fprintf(outFile, "KET QUA:\n");
                fprintf(outFile, "Tong quang duong: 0\n");
                fprintf(outFile, "Duong di: %c\n", 'A' + startNodeParam);
            }
            else { // Không có đường đi giữa hai đỉnh khác nhau
                fprintf(outFile, "KET QUA: Khong tim thay duong di giua hai dinh da chon.\n");
            }
        }
        else { // Chưa chọn đủ đỉnh hoặc chưa chạy thuật toán
            fprintf(outFile, "KET QUA: Chua chay thuat toan hoac chua chon du dinh.\n");
        }
    }
    else { // Trường hợp tìm thấy đường đi
        fprintf(outFile, "KET QUA:\n");
        fprintf(outFile, "Tong quang duong: %d\n", distParam);
        fprintf(outFile, "Duong di: ");
        if (pathCountParam > 0 && pathParam) { // Nếu có đường đi và mảng đường đi hợp lệ
            for (int i = 0; i < pathCountParam; ++i) {
                fprintf(outFile, "%c", 'A' + pathParam[i]);
                if (i < pathCountParam - 1) {
                    fprintf(outFile, " -> ");
                }
            }
            fprintf(outFile, "\n");
        }
        else if (startNodeParam == endNodeParam && distParam == 0) { // Trường hợp đặc biệt start=end, dist=0
            fprintf(outFile, "%c\n", 'A' + startNodeParam);
        }
        else { // Lỗi không mong muốn
            fprintf(outFile, "(Loi xay dung duong di hoac khong co duong di)\n");
        }
    }

    fprintf(outFile, "===================================\n");
    fclose(outFile); // Đóng file

    // Thông báo thành công cho người dùng
    char successMsg[MAX_PATH_LENGTH + 50];
    snprintf(successMsg, sizeof(successMsg), "Da xuat ket qua ra file:\n%s", filename);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Xuat File Thanh Cong", successMsg, NULL);
}

// Hàm đặt lại trạng thái của thuật toán (cho lần chạy mới hoặc reset)
void resetAlgorithmState() {
    stepExecutionActive = false; // Không còn chạy từng bước
    if (final_path) { free(final_path); final_path = NULL; } // Xóa đường đi cũ
    final_path_count = 0;
    totalDistance = INT_MAX; // Đặt lại tổng khoảng cách

    nodeToHighlight = -1; // Không có đỉnh nào được tô sáng đặc biệt
    currentDijkstraStepNode = -1; // Reset đỉnh đang xét của Dijkstra
    clear_edges_to_highlight(); // Xóa danh sách cạnh tô sáng
    bellmanNegativeCycleDetected = false; // Reset cờ chu trình âm
    currentBellmanIteration = 0; // Reset số vòng lặp Bellman-Ford

    // Giải phóng các mảng trạng thái của thuật toán từng bước
    if (dijkstraStepDist) { free(dijkstraStepDist); dijkstraStepDist = NULL; }
    if (dijkstraStepVisited) { free(dijkstraStepVisited); dijkstraStepVisited = NULL; }
    if (dijkstraStepPrev) { free(dijkstraStepPrev); dijkstraStepPrev = NULL; }
    if (bellmanStepDist) { free(bellmanStepDist); bellmanStepDist = NULL; }
    if (bellmanStepPrev) { free(bellmanStepPrev); bellmanStepPrev = NULL; }

    // Nếu đồ thị đã được tải và có đỉnh, cấp phát lại các mảng trạng thái
    if (N > 0 && graphLoaded) {
        dijkstraStepDist = (int*)malloc(N * sizeof(int));
        dijkstraStepVisited = (bool*)calloc(N, sizeof(bool)); // calloc khởi tạo là false
        dijkstraStepPrev = (int*)malloc(N * sizeof(int));
        bellmanStepDist = (int*)malloc(N * sizeof(int));
        bellmanStepPrev = (int*)malloc(N * sizeof(int));

        // Kiểm tra cấp phát thành công
        if (!dijkstraStepDist || !dijkstraStepVisited || !dijkstraStepPrev || !bellmanStepDist || !bellmanStepPrev) {
            fprintf(stderr, "Memory allocation failed in resetAlgorithmState for N=%d\n", N);
            // Giải phóng những gì đã cấp phát nếu có lỗi
            if (dijkstraStepDist) free(dijkstraStepDist); dijkstraStepDist = NULL;
            if (dijkstraStepVisited) free(dijkstraStepVisited); dijkstraStepVisited = NULL;
            if (dijkstraStepPrev) free(dijkstraStepPrev); dijkstraStepPrev = NULL;
            if (bellmanStepDist) free(bellmanStepDist); bellmanStepDist = NULL;
            if (bellmanStepPrev) free(bellmanStepPrev); bellmanStepPrev = NULL;
            sprintf(currentErrorMessage, "Loi cap phat bo nho khi reset thuat toan.");
            return; // Thoát nếu không cấp phát đủ
        }

        // Khởi tạo giá trị ban đầu cho các mảng trạng thái
        for (int i = 0; i < N; ++i) {
            dijkstraStepDist[i] = INT_MAX;
            dijkstraStepVisited[i] = false; // Thực ra calloc đã làm việc này
            dijkstraStepPrev[i] = -1;
            bellmanStepDist[i] = INT_MAX;
            bellmanStepPrev[i] = -1;
        }
    }
}

// Khởi tạo trạng thái cho thuật toán Dijkstra chạy từng bước
void initDijkstraStep(int start_node_param) {
    if (start_node_param < 0 || start_node_param >= N || !graphLoaded || N == 0) return; // Kiểm tra điều kiện

    // Đảm bảo các mảng trạng thái đã được cấp phát (gọi resetAlgorithmState nếu cần)
    if (!dijkstraStepDist || !dijkstraStepVisited || !dijkstraStepPrev) {
        resetAlgorithmState(); // Thử cấp phát lại
        if (!dijkstraStepDist || !dijkstraStepVisited || !dijkstraStepPrev) { // Kiểm tra lại
            fprintf(stderr, "Dijkstra step arrays still not initialized after reset for N=%d.\n", N);
            sprintf(currentErrorMessage, "Loi: Mang Dijkstra khong the khoi tao.");
            return;
        }
    }

    // Khởi tạo giá trị cho các mảng trạng thái Dijkstra
    for (int i = 0; i < N; ++i) {
        dijkstraStepDist[i] = INT_MAX;
        dijkstraStepVisited[i] = false;
        dijkstraStepPrev[i] = -1;
    }
    dijkstraStepDist[start_node_param] = 0; // Khoảng cách đến đỉnh bắt đầu là 0

    currentDijkstraStepNode = -1; // Chưa có đỉnh nào được chọn làm 'u' ở bước đầu tiên
    nodeToHighlight = start_node_param; // Đỉnh bắt đầu có thể được tô sáng ban đầu
    stepExecutionActive = true; // Bắt đầu chế độ chạy từng bước
    activeStepAlgorithm = DIJKSTRA; // Đặt thuật toán hiện tại là Dijkstra
    clear_edges_to_highlight(); // Xóa các cạnh tô sáng cũ
    if (final_path) { free(final_path); final_path = NULL; } // Xóa đường đi cũ
    final_path_count = 0;
    totalDistance = INT_MAX; // Reset tổng khoảng cách
    bellmanNegativeCycleDetected = false; // Dijkstra không phát hiện chu trình âm trực tiếp
}

// Thực hiện một bước của thuật toán Dijkstra
bool performDijkstraStep(int end_node_param) {
    // Kiểm tra xem có thể thực hiện bước tiếp theo không
    if (!stepExecutionActive || activeStepAlgorithm != DIJKSTRA || N == 0 || !graphLoaded ||
        !dijkstraStepDist || !dijkstraStepVisited || !dijkstraStepPrev) {
        stepExecutionActive = false; // Dừng nếu điều kiện không thỏa mãn
        return false; // Không thực hiện được
    }

    clear_edges_to_highlight(); // Xóa các cạnh tô sáng từ bước trước
    nodeToHighlight = -1; // Reset đỉnh tô sáng

    // Tìm đỉnh u chưa thăm có dist nhỏ nhất (như trong dijkstra_instant)
    int u = -1;
    int minDistVal = INT_MAX;
    for (int j = 0; j < N; ++j) {
        if (!dijkstraStepVisited[j] && dijkstraStepDist[j] < minDistVal) {
            minDistVal = dijkstraStepDist[j];
            u = j;
        }
    }

    currentDijkstraStepNode = u; // Lưu lại đỉnh u vừa chọn
    nodeToHighlight = u;       // Đỉnh u này sẽ được tô sáng

    // Nếu không tìm thấy u (tất cả các đỉnh còn lại không thể đến được)
    // hoặc dist[u] là vô cực (và u không phải đỉnh bắt đầu - trường hợp này có thể bỏ qua nếu dist[startNode]=0)
    // thì thuật toán kết thúc
    if (u == -1 || (dijkstraStepDist[u] == INT_MAX && u != startNode)) { // startNode sẽ có dist 0
        // Tính toán tổng khoảng cách cuối cùng
        totalDistance = (end_node_param >= 0 && end_node_param < N && dijkstraStepDist) ? dijkstraStepDist[end_node_param] : INT_MAX;
        if (startNode == endNode && N > 0) totalDistance = 0;

        // Xây dựng đường đi nếu tìm thấy
        if ((totalDistance != INT_MAX || (startNode == endNode && N > 0)) && dijkstraStepPrev) {
            buildPath(startNode, endNode, dijkstraStepPrev, N);
        }

        stepExecutionActive = false; // Kết thúc chạy từng bước
        currentDijkstraStepNode = -1; // Reset đỉnh đang xét

        // Kiểm tra và cảnh báo nếu có cạnh âm (Dijkstra có thể sai)
        bool hasNegative = false;
        if (graphLoaded && N > 0) {
            for (int i_ = 0; i_ < N; ++i_) {
                for (int j_ = 0; j_ < N; ++j_) {
                    if (graph[i_][j_] < 0) {
                        hasNegative = true;
                        break;
                    }
                }
                if (hasNegative) break;
            }
        }
        if (hasNegative) sprintf(currentErrorMessage, "Dijkstra hoan thanh (CANH BAO: Ket qua co the sai voi canh am!)");
        else strcpy(currentErrorMessage, ""); // Xóa thông báo lỗi nếu không có
        return false; // Báo hiệu thuật toán đã kết thúc
    }

    dijkstraStepVisited[u] = true; // Đánh dấu u đã thăm

    // Nếu u là đỉnh đích, thuật toán cũng kết thúc (tìm thấy đường đi)
    if (u == end_node_param && end_node_param != -1) {
        totalDistance = dijkstraStepDist[end_node_param];
        if ((totalDistance != INT_MAX || (startNode == endNode && N > 0)) && dijkstraStepPrev) {
            buildPath(startNode, endNode, dijkstraStepPrev, N);
        }

        stepExecutionActive = false;
        currentDijkstraStepNode = -1;
        // Kiểm tra cạnh âm
        bool hasNegative = false;
        if (graphLoaded && N > 0) {
            for (int i_ = 0; i_ < N; ++i_) {
                for (int j_ = 0; j_ < N; ++j_) {
                    if (graph[i_][j_] < 0) { hasNegative = true; break; }
                } if (hasNegative) break;
            }
        }
        if (hasNegative) sprintf(currentErrorMessage, "Dijkstra hoan thanh (CANH BAO: Ket qua co the sai voi canh am!)");
        else strcpy(currentErrorMessage, "");
        return false; // Báo hiệu thuật toán đã kết thúc
    }

    // "Relax" các cạnh kề của u
    for (int v = 0; v < N; ++v) {
        if (graph[u][v] != 0 && !dijkstraStepVisited[v] && dijkstraStepDist[u] != INT_MAX) {
            long long newDist_ll = (long long)dijkstraStepDist[u] + graph[u][v];
            if (newDist_ll < dijkstraStepDist[v]) {
                if (newDist_ll < INT_MIN || newDist_ll > INT_MAX) continue; // Kiểm tra tràn số
                dijkstraStepDist[v] = (int)newDist_ll;
                dijkstraStepPrev[v] = u;
                add_edge_to_highlight(u, v); // Thêm cạnh (u,v) vào danh sách tô sáng cho bước này
            }
        }
    }
    return true; // Báo hiệu vẫn còn bước tiếp theo
}

// Khởi tạo trạng thái cho thuật toán Bellman-Ford chạy từng bước
void initBellmanFordStep(int start_node_param) {
    if (start_node_param < 0 || start_node_param >= N || !graphLoaded || N == 0) return;

    // Đảm bảo các mảng trạng thái đã được cấp phát
    if (!bellmanStepDist || !bellmanStepPrev) {
        resetAlgorithmState();
        if (!bellmanStepDist || !bellmanStepPrev) {
            fprintf(stderr, "Bellman-Ford step arrays still not initialized after reset for N=%d.\n", N);
            sprintf(currentErrorMessage, "Loi: Mang Bellman-Ford khong the khoi tao.");
            return;
        }
    }

    // Khởi tạo giá trị
    for (int i = 0; i < N; ++i) {
        bellmanStepDist[i] = INT_MAX;
        bellmanStepPrev[i] = -1;
    }
    bellmanStepDist[start_node_param] = 0;

    currentBellmanIteration = 0; // Bắt đầu từ vòng lặp 0
    bellmanNegativeCycleDetected = false; // Reset cờ chu trình âm
    nodeToHighlight = start_node_param; // Có thể tô sáng đỉnh bắt đầu
    stepExecutionActive = true;
    activeStepAlgorithm = BELLMAN_FORD;
    clear_edges_to_highlight();
    if (final_path) { free(final_path); final_path = NULL; }
    final_path_count = 0;
    totalDistance = INT_MAX;
}

// Thực hiện một bước của thuật toán Bellman-Ford
bool performBellmanFordStep(int end_node_param) {
    // Kiểm tra điều kiện
    if (!stepExecutionActive || activeStepAlgorithm != BELLMAN_FORD || N == 0 || !graphLoaded ||
        !bellmanStepDist || !bellmanStepPrev) {
        stepExecutionActive = false; return false;
    }

    clear_edges_to_highlight(); // Xóa cạnh tô sáng từ bước trước
    nodeToHighlight = -1; // Reset đỉnh tô sáng

    // Giai đoạn 1: N-1 vòng lặp "relax" các cạnh
    if (currentBellmanIteration < N - 1) {
        bool relaxedThisIteration = false; // Cờ kiểm tra xem có cạnh nào được relax trong vòng lặp này không
        // Duyệt qua tất cả các cạnh (u, v)
        for (int u = 0; u < N; ++u) {
            if (bellmanStepDist[u] == INT_MAX) continue; // Bỏ qua nếu u chưa đến được
            for (int v = 0; v < N; ++v) {
                if (graph[u][v] != 0) { // Nếu có cạnh (u,v)
                    long long new_dist_check = (long long)bellmanStepDist[u] + graph[u][v];
                    if (new_dist_check < bellmanStepDist[v]) {
                        if (new_dist_check < INT_MIN || new_dist_check > INT_MAX) continue; // Tránh tràn số
                        bellmanStepDist[v] = (int)new_dist_check;
                        bellmanStepPrev[v] = u;
                        add_edge_to_highlight(u, v); // Tô sáng cạnh vừa được relax
                        nodeToHighlight = v; // Tô sáng đỉnh v (đích của cạnh relax)
                        relaxedThisIteration = true;
                    }
                }
            }
        }
        currentBellmanIteration++; // Tăng số vòng lặp
        return true; // Vẫn còn bước tiếp theo (trong giai đoạn này hoặc giai đoạn kiểm tra chu trình âm)
    }
    // Giai đoạn 2: Vòng lặp thứ N, kiểm tra chu trình âm
    else if (currentBellmanIteration == N - 1) {
        bellmanNegativeCycleDetected = false; // Giả sử chưa có chu trình âm
        // Duyệt qua tất cả các cạnh một lần nữa
        for (int u = 0; u < N; ++u) {
            if (bellmanStepDist[u] == INT_MAX) continue;
            for (int v = 0; v < N; ++v) {
                if (graph[u][v] != 0) {
                    long long new_dist_check = (long long)bellmanStepDist[u] + graph[u][v];
                    if (new_dist_check < bellmanStepDist[v]) {
                        if (new_dist_check >= INT_MIN && new_dist_check <= INT_MAX) { // Chỉ xét nếu không tràn
                            bellmanNegativeCycleDetected = true; // Phát hiện chu trình âm
                            add_edge_to_highlight(u, v); // Tô sáng cạnh gây ra phát hiện
                            nodeToHighlight = v; // Tô sáng đỉnh liên quan
                        }
                    }
                }
            }
            if (bellmanNegativeCycleDetected) break; // Nếu đã phát hiện thì không cần kiểm tra thêm
        }
        currentBellmanIteration++; // Đánh dấu đã hoàn thành vòng kiểm tra chu trình âm
        return true; // Vẫn còn một "bước" nữa để xử lý kết quả (trong lần gọi tiếp theo)
    }
    // Giai đoạn 3: Xử lý kết quả sau khi đã hoàn thành N vòng lặp
    else {
        stepExecutionActive = false; // Kết thúc chạy từng bước
        if (!bellmanNegativeCycleDetected) { // Nếu không có chu trình âm
            totalDistance = (end_node_param >= 0 && end_node_param < N) ? bellmanStepDist[end_node_param] : INT_MAX;
            if (startNode == endNode && N > 0) totalDistance = 0;

            if ((totalDistance != INT_MAX || (startNode == endNode && N > 0)) && bellmanStepPrev) {
                buildPath(startNode, endNode, bellmanStepPrev, N); // Xây dựng đường đi
            }
        }
        else { // Nếu có chu trình âm
            totalDistance = INT_MAX; // Không có đường đi ngắn nhất xác định
            if (final_path) { free(final_path); final_path = NULL; } // Xóa đường đi (nếu có)
            final_path_count = 0;
            sprintf(currentErrorMessage, "Phat hien chu trinh am!"); // Thông báo lỗi
        }
        return false; // Báo hiệu thuật toán đã kết thúc hoàn toàn
    }
}

// Hàm dọn dẹp tất cả các tài nguyên toàn cục đã cấp phát
void cleanup_globals() {
    free_graph_matrix(); // Giải phóng ma trận đồ thị
    if (node_positions) { free(node_positions); node_positions = NULL; } // Giải phóng mảng vị trí đỉnh
    node_positions_count = 0;

    // Giải phóng các mảng liên quan đến đường đi
    if (prev_nodes_for_path_build) { free(prev_nodes_for_path_build); prev_nodes_for_path_build = NULL; }
    prev_nodes_for_path_build_count = 0;
    if (final_path) { free(final_path); final_path = NULL; }
    final_path_count = 0;

    // Giải phóng các mảng trạng thái của thuật toán từng bước
    if (dijkstraStepDist) { free(dijkstraStepDist); dijkstraStepDist = NULL; }
    if (dijkstraStepVisited) { free(dijkstraStepVisited); dijkstraStepVisited = NULL; }
    if (dijkstraStepPrev) { free(dijkstraStepPrev); dijkstraStepPrev = NULL; }
    if (bellmanStepDist) { free(bellmanStepDist); bellmanStepDist = NULL; }
    if (bellmanStepPrev) { free(bellmanStepPrev); bellmanStepPrev = NULL; }

    // Giải phóng mảng cạnh tô sáng
    if (edgesToHighlight) { free(edgesToHighlight); edgesToHighlight = NULL; }
    edgesToHighlight_count = 0;
    edgesToHighlight_capacity = 0;

    // Reset các biến trạng thái khác
    N = 0;
    graphLoaded = false;
    startNode = -1;
    endNode = -1;
    totalDistance = INT_MAX;
    activeStepAlgorithm = NONE_ALGO;
    bellmanNegativeCycleDetected = false;
    stepExecutionActive = false;
    currentDijkstraStepNode = -1;
    currentBellmanIteration = 0;
    nodeToHighlight = -1;
    strcpy(graphFilePath, "DOTHI.txt"); // Đặt lại đường dẫn file mặc định
    strcpy(currentErrorMessage, ""); // Xóa thông báo lỗi
    currentTextInputContext = TEXT_INPUT_IDLE; // Reset ngữ cảnh nhập văn bản
    gui_input_N = 0; // Reset biến nhập liệu GUI
    gui_input_currentRow = 0;
}

// Hàm bắt đầu hiển thị hộp thoại (modal) để người dùng nhập văn bản
void start_text_input_modal(const char* prompt_text, AppState previous_state_param, TextInputContextID context_id_for_callback) {
    // Sao chép chuỗi nhắc nhở
    strncpy(textInputPrompt, prompt_text, sizeof(textInputPrompt) - 1);
    textInputPrompt[sizeof(textInputPrompt) - 1] = '\0'; // Đảm bảo kết thúc chuỗi

    globalTextInputBuffer[0] = '\0'; // Xóa bộ đệm nhập liệu cũ
    textInputPreviousState = previous_state_param; // Lưu trạng thái trước đó để quay lại
    currentAppState = APP_STATE_TEXT_INPUT_MODAL;  // Chuyển sang trạng thái nhập văn bản
    currentTextInputContext = context_id_for_callback; // Đặt ngữ cảnh cho việc xử lý sau khi nhập xong
    SDL_StartTextInput(); // Kích hoạt chế độ nhập văn bản của SDL (để nhận sự kiện SDL_TEXTINPUT)
}

// Hàm xử lý văn bản sau khi người dùng hoàn tất việc nhập trong hộp thoại
void process_completed_text_input(bool success, SDL_Window* window_ptr_for_msgbox) {
    if (!success) { // Nếu người dùng hủy (ví dụ: nhấn Esc)
        currentTextInputContext = TEXT_INPUT_IDLE; // Reset ngữ cảnh
        strcpy(currentErrorMessage, "Thao tac nhap da bi huy."); // Thông báo
        // Trạng thái ứng dụng sẽ được khôi phục ở nơi gọi (trong main event loop)
        return;
    }

    // Xử lý dựa trên ngữ cảnh nhập liệu đã được đặt trước đó
    switch (currentTextInputContext) {
    case TEXT_INPUT_AWAITING_FILENAME: // Chờ nhập tên file đồ thị
        if (strlen(globalTextInputBuffer) == 0) { // Nếu để trống, dùng tên mặc định
            strncpy(graphFilePath, "DOTHI.txt", MAX_PATH_LENGTH - 1);
        }
        else { // Nếu có nhập, dùng tên đã nhập
            strncpy(graphFilePath, globalTextInputBuffer, MAX_PATH_LENGTH - 1);
        }
        graphFilePath[MAX_PATH_LENGTH - 1] = '\0'; // Đảm bảo kết thúc chuỗi

        if (readGraphFromFile(graphFilePath)) { // Thử đọc file
            resetAlgorithmState(); // Nếu đọc thành công, reset trạng thái thuật toán
            currentAppState = APP_STATE_DISPLAY_MODE_SELECTION; // Chuyển sang chọn chế độ hiển thị
            strcpy(currentErrorMessage, ""); // Xóa lỗi cũ (nếu có)
        }
        else {
            // Nếu đọc file thất bại, readGraphFromFile đã set currentErrorMessage
            currentAppState = textInputPreviousState; // Quay lại trạng thái trước đó
        }
        currentTextInputContext = TEXT_INPUT_IDLE; // Reset ngữ cảnh
        break;

    case TEXT_INPUT_AWAITING_OUTPUT_FILENAME: // Chờ nhập tên file xuất kết quả
    {
        char outputFilename[MAX_PATH_LENGTH];
        if (strlen(globalTextInputBuffer) == 0) { // Nếu để trống, dùng tên mặc định
            strncpy(outputFilename, "ket_qua_export.txt", sizeof(outputFilename) - 1);
        }
        else { // Dùng tên đã nhập
            strncpy(outputFilename, globalTextInputBuffer, sizeof(outputFilename) - 1);
        }
        outputFilename[sizeof(outputFilename) - 1] = '\0';

        if (strlen(outputFilename) > 0) { // Nếu có tên file hợp lệ
            exportResultsToFile(outputFilename, startNode, endNode,
                totalDistance, final_path, final_path_count,
                bellmanNegativeCycleDetected, activeStepAlgorithm); // Gọi hàm xuất file
        }
        else { // Nếu tên file không hợp lệ (ví dụ, người dùng xóa hết rồi Enter)
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Huy Xuat File", "Khong co ten file duoc nhap. Huy bo xuat file.", window_ptr_for_msgbox);
        }
        currentTextInputContext = TEXT_INPUT_IDLE; // Reset ngữ cảnh
        currentAppState = textInputPreviousState; // Quay lại trạng thái hiển thị (Visualization)
    }
    break;

    case TEXT_INPUT_AWAITING_NUM_NODES: // Chờ nhập số lượng đỉnh N
    {
        int temp_n_val;
        // Thử parse số đỉnh từ chuỗi nhập, kiểm tra giới hạn (1-26, vì dùng ký tự A-Z)
        if (sscanf(globalTextInputBuffer, "%d", &temp_n_val) == 1 && temp_n_val > 0 && temp_n_val <= 26) {
            cleanup_globals(); // Dọn dẹp trạng thái cũ

            if (allocate_graph_matrix(temp_n_val)) { // Cấp phát ma trận mới
                gui_input_N = temp_n_val; // Lưu số đỉnh N cho việc nhập ma trận
                gui_input_currentRow = 0;   // Bắt đầu nhập từ hàng đầu tiên (A)

                // Tạo chuỗi nhắc cho hàng đầu tiên
                char prompt[MAX_PATH_LENGTH];
                snprintf(prompt, sizeof(prompt), "Nhap hang %c (%d so nguyen, cach nhau boi space):",
                    'A' + gui_input_currentRow, gui_input_N);
                // Gọi lại modal để nhập hàng đầu tiên
                start_text_input_modal(prompt, textInputPreviousState, TEXT_INPUT_AWAITING_MATRIX_ROW);
            }
            else { // Nếu cấp phát ma trận thất bại
                currentTextInputContext = TEXT_INPUT_IDLE;
                currentAppState = textInputPreviousState; // Quay lại (allocate_graph_matrix đã set lỗi)
            }
        }
        else { // Nếu số đỉnh nhập không hợp lệ
            sprintf(currentErrorMessage, "So dinh khong hop le: '%s'. Nhap so nguyen (1-26).", globalTextInputBuffer);
            currentTextInputContext = TEXT_INPUT_IDLE;
            currentAppState = textInputPreviousState; // Quay lại
        }
    }
    break;

    case TEXT_INPUT_AWAITING_MATRIX_ROW: // Chờ nhập một hàng của ma trận kề
    {
        // Kiểm tra trạng thái có nhất quán không (N, gui_input_N, gui_input_currentRow)
        if (N <= 0 || N != gui_input_N || gui_input_currentRow < 0 || gui_input_currentRow >= N) {
            sprintf(currentErrorMessage, "Loi trang thai nhap ma tran. Bat dau lai.");
            cleanup_globals(); // Dọn dẹp
            currentTextInputContext = TEXT_INPUT_IDLE;
            currentAppState = APP_STATE_GRAPH_INPUT_METHOD; // Quay về màn hình chọn cách nhập
            break;
        }

        char* p = globalTextInputBuffer; // Con trỏ để duyệt chuỗi nhập
        int val;
        int parsed_count = 0; // Số lượng số nguyên đã parse được từ hàng
        bool row_parse_error = false; // Cờ báo lỗi khi parse hàng

        // Parse N số nguyên từ chuỗi
        for (int j = 0; j < gui_input_N; ++j) {
            while (*p && isspace((unsigned char)*p)) p++; // Bỏ qua khoảng trắng đầu
            if (!*p && j < gui_input_N) { // Hết chuỗi sớm (thiếu số)
                row_parse_error = true;
                break;
            }
            if (!*p && j == 0 && gui_input_N > 0) { // Trường hợp nhập chuỗi rỗng cho hàng đầu tiên
                row_parse_error = true;
                break;
            }
            if (!*p) break; // Hết chuỗi, đã parse đủ (có thể không cần)

            char* end_num; // Con trỏ đến ký tự sau số đã parse
            val = strtol(p, &end_num, 10); // Parse số nguyên hệ 10
            if (p == end_num) { // Nếu không parse được số nào (ví dụ, gặp ký tự không phải số)
                row_parse_error = true;
                break;
            }
            graph[gui_input_currentRow][j] = val; // Lưu giá trị vào ma trận
            parsed_count++;
            p = end_num; // Di chuyển con trỏ p
        }

        // Sau khi parse N số, kiểm tra xem còn ký tự thừa không (trừ khoảng trắng)
        if (!row_parse_error && parsed_count == gui_input_N) {
            while (*p && isspace((unsigned char)*p)) p++; // Bỏ qua khoảng trắng cuối
            if (*p != '\0') { // Nếu còn ký tự khác khoảng trắng -> lỗi (thừa số)
                row_parse_error = true;
            }
        }

        // Nếu có lỗi parse hoặc không đủ/thừa số
        if (row_parse_error || parsed_count != gui_input_N) {
            sprintf(currentErrorMessage, "Loi hang %c: '%s'. Can %d so nguyen.",
                'A' + gui_input_currentRow, globalTextInputBuffer, gui_input_N);
            // Yêu cầu nhập lại hàng hiện tại
            char prompt[MAX_PATH_LENGTH];
            snprintf(prompt, sizeof(prompt), "Nhap LAI hang %c (%d so nguyen, cach nhau boi space):",
                'A' + gui_input_currentRow, gui_input_N);
            start_text_input_modal(prompt, textInputPreviousState, TEXT_INPUT_AWAITING_MATRIX_ROW);
        }
        else { // Nếu nhập hàng thành công
            gui_input_currentRow++; // Chuyển sang hàng tiếp theo
            if (gui_input_currentRow < gui_input_N) { // Nếu chưa đủ N hàng
                // Yêu cầu nhập hàng tiếp theo
                char prompt[MAX_PATH_LENGTH];
                snprintf(prompt, sizeof(prompt), "Nhap hang %c (%d so nguyen, cach nhau boi space):",
                    'A' + gui_input_currentRow, gui_input_N);
                start_text_input_modal(prompt, textInputPreviousState, TEXT_INPUT_AWAITING_MATRIX_ROW);
            }
            else { // Đã nhập đủ N hàng -> hoàn tất nhập ma trận
                graphLoaded = true; // Đánh dấu đồ thị đã tải
                // Cấp phát và tính toán vị trí các đỉnh (tương tự readGraphFromFile)
                node_positions = (Node*)malloc(N * sizeof(Node));
                if (!node_positions) {
                    sprintf(currentErrorMessage, "Loi cap phat vi tri dinh.");
                    cleanup_globals(); // Dọn dẹp nếu lỗi
                    currentTextInputContext = TEXT_INPUT_IDLE;
                    currentAppState = textInputPreviousState; // Quay lại
                }
                else {
                    node_positions_count = N;
                    int graphDisplayCenterX = SCREEN_WIDTH / 2 + (BUTTON_WIDTH + PANEL_PADDING) / 2;
                    int graphDisplayCenterY = SCREEN_HEIGHT / 2 - 20;
                    int R_layout_console = ((SCREEN_WIDTH - (BUTTON_WIDTH + PANEL_PADDING * 3)) / 2 < (SCREEN_HEIGHT - PANEL_PADDING * 2 - 60) / 2 ? (SCREEN_WIDTH - (BUTTON_WIDTH + PANEL_PADDING * 3)) / 2 : (SCREEN_HEIGHT - PANEL_PADDING * 2 - 60) / 2) - NODE_RADIUS;
                    R_layout_console = (R_layout_console > (N > 1 ? 80 : 0) ? R_layout_console : (N > 1 ? 80 : 0));
                    if (N == 1) R_layout_console = 0;
                    for (int k_ = 0; k_ < N; ++k_) {
                        double angle = (N == 0 || N == 1) ? -M_PI / 2.0 : (2 * M_PI * k_ / N - M_PI / 2.0);
                        node_positions[k_].x = graphDisplayCenterX + (int)(R_layout_console * cos(angle));
                        node_positions[k_].y = graphDisplayCenterY + (int)(R_layout_console * sin(angle));
                    }
                    resetAlgorithmState(); // Reset trạng thái thuật toán
                    currentAppState = APP_STATE_DISPLAY_MODE_SELECTION; // Chuyển sang chọn chế độ hiển thị
                    snprintf(currentErrorMessage, sizeof(currentErrorMessage), "Nhap ma tran thanh cong!"); // Thông báo thành công
                    currentTextInputContext = TEXT_INPUT_IDLE; // Reset ngữ cảnh
                }
            }
        }
    }
    break;

    default: // Ngữ cảnh không xác định (không nên xảy ra)
        currentTextInputContext = TEXT_INPUT_IDLE;
        break;
    }
}

// Hàm chính của chương trình
int main(int argc, char* argv[]) {
    // Khởi tạo SDL Video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    // Khởi tạo SDL_ttf
    if (TTF_Init() == -1) {
        fprintf(stderr, "SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Tạo cửa sổ
    SDL_Window* window = SDL_CreateWindow("Dijkstra and Bellman-Ford Visualizer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        TTF_Quit(); SDL_Quit();
        return 1;
    }
    // Tạo renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window); TTF_Quit(); SDL_Quit();
        return 1;
    }

    // Tải các font chữ
    TTF_Font* regularFont = TTF_OpenFont("arial.ttf", 16); // Font thường
    TTF_Font* titleFont = TTF_OpenFont("arial.ttf", 32);   // Font cho tiêu đề
    TTF_Font* subtitleFont = TTF_OpenFont("arial.ttf", 20);// Font cho phụ đề/thông tin
    TTF_Font* buttonFont = TTF_OpenFont("arial.ttf", 18);  // Font cho nút bấm
    TTF_Font* statusFont = TTF_OpenFont("arial.ttf", 20);  // Font cho text trạng thái
    // Kiểm tra tải font thành công
    if (!regularFont || !titleFont || !subtitleFont || !buttonFont || !statusFont) {
        fprintf(stderr, "Failed to load one or more fonts: %s\n", TTF_GetError());
        // Giải phóng các font đã tải (nếu có)
        if (regularFont) TTF_CloseFont(regularFont);
        if (titleFont) TTF_CloseFont(titleFont);
        if (subtitleFont) TTF_CloseFont(subtitleFont);
        if (buttonFont) TTF_CloseFont(buttonFont);
        if (statusFont) TTF_CloseFont(statusFont);
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); TTF_Quit(); SDL_Quit();
        return 1;
    }

    // Khởi tạo các nút bấm (vị trí sẽ được cập nhật trong vòng lặp chính)
    // Main Menu
    Button_init(&startButton_MainMenu, (SDL_Rect) { 0, 0, 0, 0 }, "START", GREEN_BTN, BLACK);
    Button_init(&exitButton_MainMenu, (SDL_Rect) { 0, 0, 0, 0 }, "EXIT", RED, BLACK);
    // Graph Input Method
    Button_init(&fromFileButton_GraphInput, (SDL_Rect) { 0, 0, 0, 0 }, "Doc tu file...", GREEN_BTN, BLACK);
    Button_init(&fromInputButton_GraphInput, (SDL_Rect) { 0, 0, 0, 0 }, "Nhap tu ban phim", ORANGE, BLACK);
    Button_init(&backButton_GraphInput, (SDL_Rect) { 0, 0, 0, 0 }, "Quay lai Main Menu", GRAY, BLACK);
    // Display Mode Selection
    Button_init(&instantButton_DisplayMode, (SDL_Rect) { 0, 0, 0, 0 }, "Hien thi ket qua ngay", GREEN_BTN, BLACK);
    Button_init(&stepButton_DisplayMode, (SDL_Rect) { 0, 0, 0, 0 }, "Chay tung buoc", ORANGE, BLACK);
    Button_init(&backButton_DisplayMode, (SDL_Rect) { 0, 0, 0, 0 }, "Quay lai Chon Do Thi", GRAY, BLACK);
    // Visualization Screen
    Button_init(&runDijkstraButton_Vis, (SDL_Rect) { 0, 0, 0, 0 }, "Chay Dijkstra", GRAY, BLACK); // Màu sẽ thay đổi dựa trên trạng thái
    Button_init(&runBellmanFordButton_Vis, (SDL_Rect) { 0, 0, 0, 0 }, "Chay Bellman-Ford", GRAY, BLACK);
    Button_init(&nextStepButton_Vis, (SDL_Rect) { 0, 0, 0, 0 }, "Buoc tiep theo", LIGHT_BLUE, BLACK);
    Button_init(&resetButtonUI_Vis, (SDL_Rect) { 0, 0, 0, 0 }, "Dat lai (Reset)", ORANGE, BLACK);
    Button_init(&exportButton_Vis, (SDL_Rect) { 0, 0, 0, 0 }, "Xuat ra file", GREEN_BTN, BLACK);
    Button_init(&backButton_Visualization, (SDL_Rect) { 0, 0, 0, 0 }, "Quay lai Chon Kieu", GRAY, BLACK);

    bool programIsRunning = true; // Cờ điều khiển vòng lặp chính
    SDL_Event e_loop; // Biến lưu sự kiện SDL

    // Vòng lặp chính của ứng dụng
    while (programIsRunning) {
        bool uiElementClickedThisFrame = false; // Cờ kiểm tra xem có UI nào được click trong frame này không
        // (để tránh xử lý click node nếu click button)

// Xử lý hàng đợi sự kiện
        while (SDL_PollEvent(&e_loop)) {
            if (e_loop.type == SDL_QUIT) { // Nếu người dùng đóng cửa sổ
                programIsRunning = false;
            }

            // Xử lý sự kiện cho hộp thoại nhập văn bản (nếu đang active)
            if (currentAppState == APP_STATE_TEXT_INPUT_MODAL) {
                if (e_loop.type == SDL_TEXTINPUT) { // Sự kiện nhập ký tự
                    // Nối ký tự vào buffer, kiểm tra tràn
                    if (strlen(globalTextInputBuffer) + strlen(e_loop.text.text) < sizeof(globalTextInputBuffer) - 2) { // -2 cho '_' và '\0'
                        strcat(globalTextInputBuffer, e_loop.text.text);
                    }
                }
                else if (e_loop.type == SDL_KEYDOWN) { // Sự kiện nhấn phím
                    if (e_loop.key.keysym.sym == SDLK_BACKSPACE && strlen(globalTextInputBuffer) > 0) { // Xóa lùi
                        globalTextInputBuffer[strlen(globalTextInputBuffer) - 1] = '\0';
                    }
                    else if (e_loop.key.keysym.sym == SDLK_RETURN || e_loop.key.keysym.sym == SDLK_KP_ENTER) { // Nhấn Enter
                        SDL_StopTextInput(); // Dừng chế độ nhập SDL
                        process_completed_text_input(true, window); // Xử lý text (true: thành công)
                        uiElementClickedThisFrame = true; // Đánh dấu có click UI
                    }
                    else if (e_loop.key.keysym.sym == SDLK_ESCAPE) { // Nhấn Escape
                        SDL_StopTextInput();
                        AppState stateToRestore = textInputPreviousState; // Lưu trạng thái cần quay về
                        process_completed_text_input(false, window); // Xử lý (false: hủy)
                        currentAppState = stateToRestore; // Khôi phục trạng thái
                        currentTextInputContext = TEXT_INPUT_IDLE; // Reset ngữ cảnh
                        uiElementClickedThisFrame = true;
                    }
                }
            }
            else { // Nếu không phải đang nhập text trong modal
                if (e_loop.type == SDL_MOUSEBUTTONDOWN) { // Sự kiện click chuột
                    int mouseX = e_loop.button.x;
                    int mouseY = e_loop.button.y;

                    // Xử lý click dựa trên trạng thái ứng dụng hiện tại
                    if (currentAppState == APP_STATE_MAIN_MENU) {
                        if (Button_isClicked(&startButton_MainMenu, mouseX, mouseY)) {
                            currentAppState = APP_STATE_GRAPH_INPUT_METHOD; // Chuyển sang chọn cách nhập đồ thị
                            strcpy(currentErrorMessage, ""); // Xóa lỗi
                            uiElementClickedThisFrame = true;
                        }
                        else if (Button_isClicked(&exitButton_MainMenu, mouseX, mouseY)) {
                            programIsRunning = false; // Thoát chương trình
                            uiElementClickedThisFrame = true;
                        }
                    }
                    else if (currentAppState == APP_STATE_GRAPH_INPUT_METHOD) {
                        if (Button_isClicked(&fromFileButton_GraphInput, mouseX, mouseY)) {
                            // Mở modal nhập tên file
                            start_text_input_modal("Nhap ten file do thi (de trong cho DOTHI.txt):",
                                APP_STATE_GRAPH_INPUT_METHOD, TEXT_INPUT_AWAITING_FILENAME);
                            uiElementClickedThisFrame = true;
                        }
                        else if (Button_isClicked(&fromInputButton_GraphInput, mouseX, mouseY)) {
                            gui_input_N = 0; // Reset biến nhập liệu
                            gui_input_currentRow = 0;
                            strcpy(currentErrorMessage, "");
                            // Mở modal nhập số đỉnh N
                            start_text_input_modal("Nhap so dinh N (1-26):",
                                APP_STATE_GRAPH_INPUT_METHOD, TEXT_INPUT_AWAITING_NUM_NODES);
                            uiElementClickedThisFrame = true;
                        }
                        else if (Button_isClicked(&backButton_GraphInput, mouseX, mouseY)) {
                            cleanup_globals(); // Dọn dẹp nếu người dùng quay lại từ đây mà chưa tải đồ thị
                            currentAppState = APP_STATE_MAIN_MENU; // Quay lại Main Menu
                            strcpy(currentErrorMessage, "");
                            uiElementClickedThisFrame = true;
                        }
                    }
                    else if (currentAppState == APP_STATE_DISPLAY_MODE_SELECTION) {
                        if (Button_isClicked(&instantButton_DisplayMode, mouseX, mouseY)) {
                            currentDisplayMode = INSTANT_RESULT; // Chọn chế độ kết quả ngay
                            currentAppState = APP_STATE_VISUALIZATION; // Chuyển sang màn hình hiển thị
                            startNode = -1; endNode = -1; activeStepAlgorithm = NONE_ALGO; // Reset chọn đỉnh/thuật toán
                            resetAlgorithmState(); // Reset trạng thái thuật toán (quan trọng vì đồ thị đã tải)
                            strcpy(currentErrorMessage, "");
                            uiElementClickedThisFrame = true;
                        }
                        else if (Button_isClicked(&stepButton_DisplayMode, mouseX, mouseY)) {
                            currentDisplayMode = STEP_BY_STEP; // Chọn chế độ từng bước
                            currentAppState = APP_STATE_VISUALIZATION;
                            startNode = -1; endNode = -1; activeStepAlgorithm = NONE_ALGO;
                            resetAlgorithmState();
                            strcpy(currentErrorMessage, "");
                            uiElementClickedThisFrame = true;
                        }
                        else if (Button_isClicked(&backButton_DisplayMode, mouseX, mouseY)) {
                            currentAppState = APP_STATE_GRAPH_INPUT_METHOD; // Quay lại chọn cách nhập đồ thị
                            strcpy(currentErrorMessage, "");
                            // Không cần cleanup_globals() ở đây vì đồ thị có thể đã được tải và vẫn muốn giữ nó
                            uiElementClickedThisFrame = true;
                        }
                    }
                    else if (currentAppState == APP_STATE_VISUALIZATION) {
                        // Nút Quay Lại
                        if (Button_isClicked(&backButton_Visualization, mouseX, mouseY)) {
                            currentAppState = APP_STATE_DISPLAY_MODE_SELECTION; // Quay lại chọn chế độ hiển thị
                            resetAlgorithmState(); // Reset trạng thái thuật toán khi rời màn hình này
                            startNode = -1; endNode = -1; activeStepAlgorithm = NONE_ALGO; // Reset chọn đỉnh
                            strcpy(currentErrorMessage, "");
                            uiElementClickedThisFrame = true;
                        }
                        // Nút Reset
                        else if (Button_isClicked(&resetButtonUI_Vis, mouseX, mouseY)) {
                            startNode = -1; endNode = -1; // Bỏ chọn đỉnh
                            resetAlgorithmState(); // Reset trạng thái thuật toán
                            activeStepAlgorithm = NONE_ALGO; // Bỏ chọn thuật toán
                            strcpy(currentErrorMessage, "");
                            uiElementClickedThisFrame = true;
                        }
                        // Nút Chạy Dijkstra
                        else if (Button_isClicked(&runDijkstraButton_Vis, mouseX, mouseY) && graphLoaded && startNode != -1 && endNode != -1) {
                            resetAlgorithmState(); activeStepAlgorithm = DIJKSTRA; // Đặt thuật toán, reset trạng thái cũ
                            if (currentDisplayMode == INSTANT_RESULT) { // Chế độ kết quả ngay
                                dijkstra_instant(startNode, endNode); // Chạy thuật toán
                                if ((totalDistance != INT_MAX || (startNode == endNode && N > 0)) && prev_nodes_for_path_build) {
                                    buildPath(startNode, endNode, prev_nodes_for_path_build, N); // Xây dựng đường đi
                                }
                                else { if (final_path) free(final_path); final_path = NULL; final_path_count = 0; }
                                // Kiểm tra cạnh âm và cảnh báo
                                bool hasNegative = false; if (graphLoaded && N > 0) { for (int i_ = 0; i_ < N; ++i_) { for (int j_ = 0; j_ < N; ++j_) { if (graph[i_][j_] < 0) { hasNegative = true; break; } }if (hasNegative)break; } }
                                if (hasNegative) sprintf(currentErrorMessage, "Dijkstra hoan thanh (CANH BAO: Ket qua co the sai voi canh am!)"); else strcpy(currentErrorMessage, "");
                            }
                            else { // Chế độ từng bước
                                initDijkstraStep(startNode); // Khởi tạo cho Dijkstra từng bước
                                strcpy(currentErrorMessage, "");
                            }
                            uiElementClickedThisFrame = true;
                        }
                        // Nút Chạy Bellman-Ford
                        else if (Button_isClicked(&runBellmanFordButton_Vis, mouseX, mouseY) && graphLoaded && startNode != -1 && endNode != -1) {
                            resetAlgorithmState(); activeStepAlgorithm = BELLMAN_FORD;
                            if (currentDisplayMode == INSTANT_RESULT) {
                                bellmanFord_instant(startNode, endNode);
                                if (!bellmanNegativeCycleDetected && (totalDistance != INT_MAX || (startNode == endNode && N > 0)) && prev_nodes_for_path_build) {
                                    buildPath(startNode, endNode, prev_nodes_for_path_build, N);
                                }
                                else { // Nếu có chu trình âm hoặc không có đường đi
                                    if (final_path) free(final_path); final_path = NULL; final_path_count = 0;
                                    if (bellmanNegativeCycleDetected) sprintf(currentErrorMessage, "Phat hien chu trinh am!");
                                    else if (totalDistance == INT_MAX && !(startNode == endNode && N > 0)) sprintf(currentErrorMessage, "Khong tim thay duong di!");
                                    else strcpy(currentErrorMessage, "");
                                }
                            }
                            else {
                                initBellmanFordStep(startNode); // Khởi tạo cho Bellman-Ford từng bước
                                strcpy(currentErrorMessage, "");
                            }
                            uiElementClickedThisFrame = true;
                        }
                        // Nút Bước Tiếp Theo (cho chế độ từng bước)
                        else if (currentDisplayMode == STEP_BY_STEP && stepExecutionActive && Button_isClicked(&nextStepButton_Vis, mouseX, mouseY)) {
                            if (activeStepAlgorithm == DIJKSTRA) performDijkstraStep(endNode);
                            else if (activeStepAlgorithm == BELLMAN_FORD) performBellmanFordStep(endNode);
                            uiElementClickedThisFrame = true;
                        }
                        // Nút Xuất Ra File
                        else if (Button_isClicked(&exportButton_Vis, mouseX, mouseY)) {
                            // Chỉ xuất khi đã có kết quả (chạy xong thuật toán, không phải đang chạy từng bước)
                            bool hasResultsToExport = (activeStepAlgorithm != NONE_ALGO && startNode != -1 && endNode != -1 && !stepExecutionActive);
                            if (hasResultsToExport) {
                                start_text_input_modal("Nhap ten file xuat (de trong cho ket_qua_export.txt):",
                                    APP_STATE_VISUALIZATION, TEXT_INPUT_AWAITING_OUTPUT_FILENAME);
                            }
                            else { // Thông báo nếu chưa có kết quả
                                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Khong co ket qua", "Chon diem bat dau, ket thuc, chay mot thuat toan (va hoan thanh neu chay tung buoc) truoc khi xuat.", window);
                            }
                            uiElementClickedThisFrame = true;
                        }

                        // Xử lý click chuột lên đỉnh để chọn đỉnh bắt đầu/kết thúc
                        // Chỉ khi không có nút nào được click và không đang chạy từng bước
                        if (!uiElementClickedThisFrame && !stepExecutionActive && graphLoaded && N > 0 && node_positions) {
                            for (int i = 0; i < node_positions_count; ++i) {
                                int dx_mouse_node = mouseX - node_positions[i].x;
                                int dy_mouse_node = mouseY - node_positions[i].y;
                                // Kiểm tra click có nằm trong bán kính đỉnh không
                                if (dx_mouse_node * dx_mouse_node + dy_mouse_node * dy_mouse_node <= NODE_RADIUS * NODE_RADIUS) {
                                    if (startNode == -1) { // Nếu chưa có đỉnh bắt đầu -> chọn đỉnh này làm bắt đầu
                                        startNode = i;
                                        endNode = -1; // Reset đỉnh kết thúc
                                    }
                                    else if (endNode == -1 && i != startNode) { // Nếu đã có bắt đầu, chưa có kết thúc -> chọn làm kết thúc
                                        endNode = i;
                                    }
                                    else { // Nếu đã có cả hai, hoặc click lại đỉnh bắt đầu -> reset, chọn lại từ đầu
                                        startNode = i;
                                        endNode = -1;
                                    }
                                    // Khi chọn đỉnh mới, reset trạng thái thuật toán và thông báo
                                    resetAlgorithmState(); activeStepAlgorithm = NONE_ALGO; strcpy(currentErrorMessage, "");
                                    break; // Đã xử lý click lên một đỉnh
                                }
                            }
                        }
                    }
                }
            }
        } // Kết thúc vòng lặp SDL_PollEvent

        // === PHẦN VẼ (RENDERING) ===
        SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a); // Màu nền trắng
        SDL_RenderClear(renderer); // Xóa màn hình bằng màu nền

        // Vẽ giao diện dựa trên trạng thái ứng dụng
        if (currentAppState == APP_STATE_MAIN_MENU) {
            int titleY = SCREEN_HEIGHT / 4 - 60; // Vị trí Y cho tiêu đề
            // Vẽ các dòng chữ tiêu đề và thông tin
            drawText(renderer, titleFont, "DE TAI : TIM DUONG DI NGAN NHAT ", SCREEN_WIDTH / 2, titleY - 50, BLACK, true, 0);
            drawText(renderer, titleFont, "DIJKSTRA & BELLMAN-FORD VISUALIZER", SCREEN_WIDTH / 2, titleY, BLACK, true, 0);
            int currentInfoY = titleY + 60;
            drawText(renderer, subtitleFont, "SVTH1: Nguyen Huu Thai - MSSV: 102240338", SCREEN_WIDTH / 2, currentInfoY, BLACK, true, 0);
            currentInfoY += 30;
            drawText(renderer, subtitleFont, "SVTH2: Hoang Dinh Chien Thang - MSSV: 102240340", SCREEN_WIDTH / 2, currentInfoY, BLACK, true, 0);
            currentInfoY += 40;
            drawText(renderer, subtitleFont, "GVHD: Nguyen Van Hieu", SCREEN_WIDTH / 2, currentInfoY, BLACK, true, 0);

            // Đặt vị trí và vẽ các nút
            int buttonStartY = currentInfoY + 60;
            startButton_MainMenu.rect = (SDL_Rect){ SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, buttonStartY, BUTTON_WIDTH, BUTTON_HEIGHT };
            Button_draw(&startButton_MainMenu, renderer, buttonFont);
            exitButton_MainMenu.rect = (SDL_Rect){ SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2, buttonStartY + BUTTON_HEIGHT + 15, BUTTON_WIDTH, BUTTON_HEIGHT };
            Button_draw(&exitButton_MainMenu, renderer, buttonFont);
        }
        else if (currentAppState == APP_STATE_GRAPH_INPUT_METHOD) {
            drawText(renderer, titleFont, "Chon cach nhap do thi:", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 - 20, BLACK, true, 0);
            int btnWidth = BUTTON_WIDTH + 60; // Nút rộng hơn một chút
            int btnX = SCREEN_WIDTH / 2 - btnWidth / 2; // Canh giữa
            int btnStartY = SCREEN_HEIGHT / 2 - BUTTON_HEIGHT * 1.5 - 15; // Vị trí Y bắt đầu của cụm nút
            fromFileButton_GraphInput.rect = (SDL_Rect){ btnX, btnStartY, btnWidth, BUTTON_HEIGHT };
            Button_draw(&fromFileButton_GraphInput, renderer, buttonFont);
            fromInputButton_GraphInput.rect = (SDL_Rect){ btnX, btnStartY + BUTTON_HEIGHT + 15, btnWidth, BUTTON_HEIGHT };
            Button_draw(&fromInputButton_GraphInput, renderer, buttonFont);
            backButton_GraphInput.rect = (SDL_Rect){ btnX, btnStartY + 2 * (BUTTON_HEIGHT + 15), btnWidth, BUTTON_HEIGHT };
            Button_draw(&backButton_GraphInput, renderer, buttonFont);
        }
        else if (currentAppState == APP_STATE_DISPLAY_MODE_SELECTION) {
            drawText(renderer, titleFont, "Chon kieu hien thi:", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 - 20, BLACK, true, 0);
            int btnWidth = BUTTON_WIDTH + 40;
            int btnX = SCREEN_WIDTH / 2 - btnWidth / 2;
            int btnStartY = SCREEN_HEIGHT / 2 - BUTTON_HEIGHT * 1.5 - 15;
            instantButton_DisplayMode.rect = (SDL_Rect){ btnX, btnStartY, btnWidth, BUTTON_HEIGHT };
            Button_draw(&instantButton_DisplayMode, renderer, buttonFont);
            stepButton_DisplayMode.rect = (SDL_Rect){ btnX, btnStartY + BUTTON_HEIGHT + 15, btnWidth, BUTTON_HEIGHT };
            Button_draw(&stepButton_DisplayMode, renderer, buttonFont);
            backButton_DisplayMode.rect = (SDL_Rect){ btnX, btnStartY + 2 * (BUTTON_HEIGHT + 15), btnWidth, BUTTON_HEIGHT };
            Button_draw(&backButton_DisplayMode, renderer, buttonFont);
        }
        else if (currentAppState == APP_STATE_VISUALIZATION) {
            int currentButtonY = PANEL_PADDING; // Y bắt đầu cho cột nút bên trái
            bool canRunAlgo = graphLoaded && startNode != -1 && endNode != -1; // Điều kiện để có thể chạy thuật toán

            // Cập nhật màu nút dựa trên trạng thái (có thể chạy được không)
            runDijkstraButton_Vis.color = canRunAlgo ? BLUE : GRAY;
            runBellmanFordButton_Vis.color = canRunAlgo ? BLUE : GRAY;

            // Vẽ các nút điều khiển thuật toán
            runDijkstraButton_Vis.rect = (SDL_Rect){ PANEL_PADDING, currentButtonY, BUTTON_WIDTH, BUTTON_HEIGHT };
            Button_draw(&runDijkstraButton_Vis, renderer, buttonFont);
            currentButtonY += BUTTON_HEIGHT + 10; // Khoảng cách giữa các nút

            runBellmanFordButton_Vis.rect = (SDL_Rect){ PANEL_PADDING, currentButtonY, BUTTON_WIDTH, BUTTON_HEIGHT };
            Button_draw(&runBellmanFordButton_Vis, renderer, buttonFont);
            currentButtonY += BUTTON_HEIGHT + 10;

            if (currentDisplayMode == STEP_BY_STEP) { // Chỉ vẽ nút "Bước tiếp theo" nếu ở chế độ từng bước
                nextStepButton_Vis.color = stepExecutionActive ? LIGHT_BLUE : GRAY; // Màu tùy thuộc đang chạy hay không
                nextStepButton_Vis.rect = (SDL_Rect){ PANEL_PADDING, currentButtonY, BUTTON_WIDTH, BUTTON_HEIGHT };
                Button_draw(&nextStepButton_Vis, renderer, buttonFont);
                currentButtonY += BUTTON_HEIGHT + 10;
            }

            resetButtonUI_Vis.rect = (SDL_Rect){ PANEL_PADDING, currentButtonY, BUTTON_WIDTH, BUTTON_HEIGHT };
            Button_draw(&resetButtonUI_Vis, renderer, buttonFont);
            currentButtonY += BUTTON_HEIGHT + 10;

            bool canExport = (activeStepAlgorithm != NONE_ALGO && startNode != -1 && endNode != -1 && !stepExecutionActive);
            exportButton_Vis.color = canExport ? GREEN_BTN : GRAY; // Màu tùy thuộc có kết quả để xuất không
            exportButton_Vis.rect = (SDL_Rect){ PANEL_PADDING, currentButtonY, BUTTON_WIDTH, BUTTON_HEIGHT };
            Button_draw(&exportButton_Vis, renderer, buttonFont);
            currentButtonY += BUTTON_HEIGHT + 10;

            backButton_Visualization.rect = (SDL_Rect){ PANEL_PADDING, currentButtonY, BUTTON_WIDTH, BUTTON_HEIGHT };
            Button_draw(&backButton_Visualization, renderer, buttonFont);

            // Vẽ đồ thị
            if (graphLoaded) {
                drawGraph(renderer, regularFont, statusFont); // Hàm này vẽ cả đỉnh, cạnh, và thông tin trạng thái/kết quả
            }
            else { // Nếu chưa có đồ thị
                drawText(renderer, titleFont, "Khong co do thi nao duoc tai.", SCREEN_WIDTH / 2 + (BUTTON_WIDTH + PANEL_PADDING) / 2, SCREEN_HEIGHT / 2 - 20, RED, true, 0);
                drawText(renderer, regularFont, "Vui long quay lai va tai/nhap do thi.", SCREEN_WIDTH / 2 + (BUTTON_WIDTH + PANEL_PADDING) / 2, SCREEN_HEIGHT / 2 + 20, RED, true, 0);
            }
        }

        // Hiển thị thông báo lỗi chung (nếu có) ở cuối màn hình
        bool messageCoveredByBottomStatus = false; // Cờ kiểm tra xem thông báo lỗi có bị trùng với thông báo kết quả ở drawGraph không
        // Kiểm tra nếu thông báo lỗi hiện tại là một trong các thông báo đã được drawGraph xử lý
        if (currentAppState == APP_STATE_VISUALIZATION && !stepExecutionActive && strlen(currentErrorMessage) > 0) {
            if (((totalDistance == INT_MAX && !(startNode == endNode && N > 0) && !bellmanNegativeCycleDetected && activeStepAlgorithm != NONE_ALGO) &&
                strcmp(currentErrorMessage, "Khong tim thay duong di!") == 0) ||
                ((bellmanNegativeCycleDetected && activeStepAlgorithm == BELLMAN_FORD) &&
                    strcmp(currentErrorMessage, "Phat hien chu trinh am!") == 0)) {
                messageCoveredByBottomStatus = true;
            }
            // Tương tự cho cảnh báo cạnh âm của Dijkstra
            if (strstr(currentErrorMessage, "CANH BAO: Ket qua co the sai voi canh am!") != NULL && activeStepAlgorithm == DIJKSTRA) {
                messageCoveredByBottomStatus = true;
            }
        }

        // Chỉ vẽ currentErrorMessage nếu nó không rỗng, không phải đang ở modal, và không bị trùng lắp
        if (strlen(currentErrorMessage) > 0 && currentAppState != APP_STATE_TEXT_INPUT_MODAL && !messageCoveredByBottomStatus) {
            int errorTextY = SCREEN_HEIGHT - PANEL_PADDING - 25; // Vị trí Y mặc định cho lỗi
            // Điều chỉnh vị trí Y nếu đang ở màn hình Visualization để không đè lên thông tin kết quả
            if (currentAppState == APP_STATE_VISUALIZATION) {
                int visBottomLineHeight = 0;
                TTF_SizeText(regularFont, "Tg", NULL, &visBottomLineHeight); // Lấy chiều cao dòng
                visBottomLineHeight = visBottomLineHeight > 0 ? visBottomLineHeight + 2 : 20;
                // Nếu có thông tin kết quả (2 dòng) hoặc thông báo chọn đỉnh (1 dòng) thì đẩy lỗi lên trên
                if (!stepExecutionActive && ((final_path_count > 0 && totalDistance != INT_MAX) || (startNode != -1 && endNode != -1 && activeStepAlgorithm != NONE_ALGO))) {
                    errorTextY -= visBottomLineHeight * 2; // Đã có 2 dòng thông tin kết quả
                }
                else if (startNode == -1 || (endNode == -1 && startNode != -1)) { // Đã có 1 dòng thông báo chọn đỉnh
                    errorTextY -= visBottomLineHeight;
                }
            }
            drawText(renderer, regularFont, currentErrorMessage, SCREEN_WIDTH / 2, errorTextY, RED, true, SCREEN_WIDTH - 40);
        }

        // Vẽ hộp thoại nhập văn bản (nếu đang active)
        if (currentAppState == APP_STATE_TEXT_INPUT_MODAL) {
            // Vẽ lớp phủ mờ phía sau hộp thoại
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Bật chế độ blend alpha
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 150); // Màu xám mờ
            SDL_Rect overlayRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
            SDL_RenderFillRect(renderer, &overlayRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE); // Tắt blend

            // Kích thước và vị trí hộp thoại
            int boxWidth = SCREEN_WIDTH / 2; // Chiều rộng bằng nửa màn hình
            int boxHeight = 150; // Chiều cao cố định
            SDL_Rect textInputBoxRect = { SCREEN_WIDTH / 2 - boxWidth / 2, SCREEN_HEIGHT / 2 - boxHeight / 2, boxWidth, boxHeight };

            // Vẽ nền và viền hộp thoại
            SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255); // Màu nền xám nhạt
            SDL_RenderFillRect(renderer, &textInputBoxRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Viền đen
            SDL_RenderDrawRect(renderer, &textInputBoxRect);

            // Vẽ chuỗi nhắc nhở
            drawText(renderer, regularFont, textInputPrompt, textInputBoxRect.x + 10, textInputBoxRect.y + 10, BLACK, false, boxWidth - 20);

            // Vẽ ô nhập liệu
            SDL_Rect inputFieldRect = { textInputBoxRect.x + 10, textInputBoxRect.y + 50, boxWidth - 20, 30 };
            SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a); // Nền trắng cho ô nhập
            SDL_RenderFillRect(renderer, &inputFieldRect);
            SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a); // Viền đen cho ô nhập
            SDL_RenderDrawRect(renderer, &inputFieldRect);

            // Vẽ văn bản đang nhập kèm con trỏ nhấp nháy (dấu '_')
            char displayTextWithCursor[sizeof(globalTextInputBuffer) + 2]; // +2 cho '_' và '\0'
            snprintf(displayTextWithCursor, sizeof(displayTextWithCursor), "%s_", globalTextInputBuffer);
            drawText(renderer, regularFont, displayTextWithCursor, inputFieldRect.x + 5, inputFieldRect.y + 5, BLACK, false, inputFieldRect.w - 10);

            // Vẽ hướng dẫn sử dụng
            drawText(renderer, regularFont, "Nhan Enter de xac nhan, Esc de huy.", textInputBoxRect.x + 10, textInputBoxRect.y + boxHeight - 30, BLACK, false, boxWidth - 20);
        }

        SDL_RenderPresent(renderer); // Hiển thị tất cả những gì đã vẽ lên màn hình
        SDL_Delay(16); // Chờ khoảng 16ms (tương đương ~60 FPS), giúp giảm tải CPU
    } // Kết thúc vòng lặp chính

    // Dọn dẹp tài nguyên trước khi thoát
    cleanup_globals(); // Giải phóng bộ nhớ đồ thị và các biến toàn cục khác
    // Đóng font
    TTF_CloseFont(regularFont); TTF_CloseFont(titleFont); TTF_CloseFont(subtitleFont); TTF_CloseFont(buttonFont); TTF_CloseFont(statusFont);
    // Hủy renderer và window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    // Đóng SDL_ttf và SDL
    TTF_Quit();
    SDL_Quit();
    return 0; // Kết thúc chương trình
}
