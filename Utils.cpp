
// Binary search to search for the required version from the sorted versions array.

int binarySearch(vector<Version>& versions, int refTime) {
    
    int left = 0, right = versions.size();
    while (left < right) {
        int mid = (left + right) / 2;
        if (versions[mid].created_at <= refTime) {
            left = mid + 1;
        }
        else {
            right = mid;
        }
    }

    return left;
}