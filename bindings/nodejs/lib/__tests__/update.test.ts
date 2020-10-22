import * as fs from "fs";
import { isUpdate, isUpdateDashboard } from "../update";
import { UpdateDashboard } from "../types";

const testFile = __dirname + "/__data__/dashboard-update-test.json";
const update = JSON.parse(fs.readFileSync(testFile, "utf-8"));
const updateGood = {
  type: "firmware",
  size: 0,
  offset: 0,
  payload: "",
  md5: "",
};
const updateBad = {
  type: "foobar",
  size: 0,
  offset: 0,
  payload: "",
  md5: "",
};

test("isUpdate", () => {
  let ret = isUpdate(updateBad);
  expect(ret).toBeFalsy();
  ret = isUpdate(updateGood);
  expect(ret).toBeTruthy();
});

test("isDashboardUpdate should pass with real file", () => {
  expect(isUpdateDashboard(update)).toBeTruthy();
});

test("isDashboardUpdate should pass with empty array", () => {
  const update = { files: [{ update: [] }] };
  expect(isUpdateDashboard(update)).toBeTruthy();
});

test("isDashboardUpdate should pass with good file", () => {
  const update = { files: [{ update: [{ ...updateGood }] }] };
  expect(isUpdateDashboard(update)).toBeTruthy();
});

test("isDashboardUpdate should fail with bad file", () => {
  const update = { files: [{ update: [{ ...updateBad }] }] };
  expect(isUpdateDashboard(update)).toBeFalsy();
});

test("isDashboardUpdate should fail with currupt data in file", () => {
  const u: any = { ...update };
  expect(isUpdateDashboard(update)).toBeTruthy();
  u.files[0].update[10].type = "foobar";
  expect(isUpdateDashboard(update)).toBeFalsy();
});
